/*
 * ast.c: Support routines for put/get
 *
 * Copyright (C) 2008-2011 David Lutterkort
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: David Lutterkort <dlutter@redhat.com>
 */

#include <config.h>
#include <stdint.h>

#include "internal.h"
#include "memory.h"
#include "lens.h"

/* A dictionary that maps key to a list of (skel, dict) */
struct dict_entry {
    struct dict_entry *next;
    struct skel *skel;
    struct dict *dict;
};

/* Associates a KEY with a list of skel/dict pairs.

   Dicts are used in two phases: first they are constructed, through
   repeated calls to dict_append. In the second phase, items are looked up
   and removed from the dict.

   During construction, MARK points to the end of the list ENTRY. Once
   construction is done, MARK points to the head of that list, and ENTRY is
   moved towards the tail everytime an item is looked up.
*/
struct dict_node {
    char *key;
    struct dict_entry *entry; /* This will change as entries are looked up */
    struct dict_entry *mark;  /* Pointer to initial entry, will never change */
};

/* Nodes are kept sorted by their key, with NULL being smaller than any
   string */
struct dict {
    struct dict_node **nodes;
    uint32_t          size;
    uint32_t          used;
    bool              marked;
};

static const int dict_initial_size = 2;
static const int dict_max_expansion = 128;
static const uint32_t dict_max_size = (1<<24) - 1;

struct dict *make_dict(char *key, struct skel *skel, struct dict *subdict) {
    struct dict *dict = NULL;
    if (ALLOC(dict) < 0)
        goto error;
    if (ALLOC_N(dict->nodes, dict_initial_size) < 0)
        goto error;
    if (ALLOC(dict->nodes[0]) < 0)
        goto error;
    if (ALLOC(dict->nodes[0]->entry) < 0)
        goto error;

    dict->size = dict_initial_size;
    dict->used = 1;
    dict->nodes[0]->key = key;
    dict->nodes[0]->entry->skel = skel;
    dict->nodes[0]->entry->dict = subdict;
    dict->nodes[0]->mark = dict->nodes[0]->entry;

    return dict;
 error:
    if (dict->nodes) {
        if (dict->nodes[0])
            FREE(dict->nodes[0]->entry);
        FREE(dict->nodes[0]);
    }
    FREE(dict->nodes);
    FREE(dict);
    return NULL;
}

void free_dict(struct dict *dict) {
    if (dict == NULL)
        return;

    for (int i=0; i < dict->used; i++) {
        struct dict_node *node = dict->nodes[i];
        if (! dict->marked)
            node->mark = node->entry;
        while (node->mark != NULL) {
            struct dict_entry *del = node->mark;
            node->mark = del->next;
            free_skel(del->skel);
            free_dict(del->dict);
            free(del);
        }
        free(node->key);
        FREE(node);
    }
    FREE(dict->nodes);
    FREE(dict);
}

/* Return the position of KEY in DICT as an integer between 0 and
   DICT->USED.  If KEY is not in DICT, return a negative number P such that
   -(P + 1) is the position at which KEY must be inserted to keep the keys
   of the nodes in DICT sorted.
*/
static int dict_pos(struct dict *dict, const char *key) {
    if (key == NULL) {
        return (dict->nodes[0]->key == NULL) ? 0 : -1;
    }

    int l = dict->nodes[0]->key == NULL ? 1 : 0;
    int h = dict->used;
    while (l < h) {
        int m = (l + h)/2;
        int cmp = strcmp(dict->nodes[m]->key, key);
        if (cmp > 0)
            h = m;
        else if (cmp < 0)
            l = m + 1;
        else
            return m;
    }
    return -(l + 1);
}

static int dict_expand(struct dict *dict) {
    uint32_t size = dict->size;

    if (size == dict_max_size)
        return -1;
    if (size > dict_max_expansion)
        size += dict_max_expansion;
    else
        size *= 2;
    if (size > dict_max_size)
        size = dict_max_size;
    dict->size = size;
    return REALLOC_N(dict->nodes, dict->size);
}

int dict_append(struct dict **dict, struct dict *d2) {
    if (d2 == NULL)
        return 0;

    if (*dict == NULL) {
        *dict = d2;
        return 0;
    }

    struct dict *d1 = *dict;
    for (int i2 = 0; i2 < d2->used; i2++) {
        struct dict_node *n2 = d2->nodes[i2];
        int i1 = dict_pos(d1, n2->key);
        if (i1 < 0) {
            i1 = - i1 - 1;
            if (d1->size == d1->used) {
                if (dict_expand(d1) < 0)
                    return -1;
            }
            memmove(d1->nodes + i1 + 1, d1->nodes + i1,
                    sizeof(*d1->nodes) * (d1->used - i1));
            d1->nodes[i1] = n2;
            d1->used += 1;
        } else {
            struct dict_node *n1 = d1->nodes[i1];
            list_tail_cons(n1->entry, n1->mark, n2->entry);
            FREE(n2->key);
            FREE(n2);
        }
    }
    FREE(d2->nodes);
    FREE(d2);
    return 0;
}

void dict_lookup(const char *key, struct dict *dict,
                 struct skel **skel, struct dict **subdict) {
    *skel = NULL;
    *subdict = NULL;
    if (dict != NULL) {
        if (! dict->marked) {
            for (int i=0; i < dict->used; i++) {
                dict->nodes[i]->mark = dict->nodes[i]->entry;
            }
            dict->marked = 1;
        }
        int p = dict_pos(dict, key);
        if (p >= 0) {
            struct dict_node *node = dict->nodes[p];
            if (node->entry != NULL) {
                *skel = node->entry->skel;
                *subdict = node->entry->dict;
                node->entry = node->entry->next;
            }
        }
    }
}



/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

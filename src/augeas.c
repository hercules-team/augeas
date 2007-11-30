/*
 * augeas.c: the core data structure for storing key/value pairs
 *
 * Copyright (C) 2007 Red Hat Inc.
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

#include "augeas.h"
#include "internal.h"

/* Statically include providers for now */
#include "prov_pam.h"

#define SEP '/'

/* Two special entries: they are always on the main list
   so that we don't need to worry about some corner cases in dealing
   with empty lists */

#define P_SYSTEM "/system"
#define P_SYSTEM_CONFIG "/system/config"

/* Doubly-linked list of path/value pairs. The list contains
   siblings in the order in which they were created, but is
   unordered otherwise */
struct aug_entry {
    const char *path;
    const char *value;
    struct aug_entry *next, *prev;
};

static struct aug_entry *head = NULL;

/* Length of PATH without any trailing '/' */
static int pathlen(const char *path) {
    int len = strlen(path);

    if (len > 0 && path[len-1] == SEP)
        len--;
    
    return len;
}

static struct aug_entry *aug_entry_find(const char *path) {
    struct aug_entry *p = head;
    do {
        if (STREQLEN(path, p->path, pathlen(path))
            && p->path[pathlen(path)] == '\0') {
            return p;
        }
        p = p->next;
    } while (p != head);
    
    return NULL;
}

static struct aug_entry *aug_entry_insert(const char *path, 
                                          struct aug_entry *next) {
    struct aug_entry *e;

    e = calloc(1, sizeof(struct aug_entry));
    if (e == NULL)
        return NULL;

    e->path = strdup(path);
    if (e->path == NULL) {
        free(e);
        return NULL;
    }

    e->next = next;
    e->prev = next->prev;

    e->next->prev = e;
    e->prev->next = e;

    return e;
}

static struct aug_entry *aug_entry_make(const char *p, 
                                        struct aug_entry *next) {
    char *path;
    path = strdupa(p);
    path[pathlen(path)] = '\0';

    for (char *pos = path+1; *pos != '\0'; pos++) {
        if (*pos == SEP) {
            *pos = '\0';
            if (aug_entry_find(path) == NULL) {
                if (aug_entry_insert(path, head) == NULL)
                    return NULL;
            }
            *pos = SEP;
        }
    }
    return aug_entry_insert(path, next);
}

static void aug_entry_free(struct aug_entry *e) {
    e->prev->next = e->next;
    e->next->prev = e->prev;

    free((char *) e->path);
    if (e->value != NULL)
        free((char *) e->value);
    free(e);
}

int aug_init(void) {
    struct aug_entry *e;
    int r;

    if (head == NULL) {
        head = calloc(1, sizeof(struct aug_entry));
        e = calloc(1, sizeof(struct aug_entry));
        if (head == NULL || e == NULL)
            return -1;
        head->path = P_SYSTEM;
        head->value = NULL;
        head->next = e;
        head->prev = e;

        e->path = P_SYSTEM_CONFIG;
        e->value = NULL;
        e->next = head;
        e->prev = head;
    }
    /* Load providers, hardcoded for now */
    r = augp_pam.init();
    if (r == -1)
        return -1;
    r = augp_pam.load();
    return r;
}

const char *aug_get(const char *path) {
    struct aug_entry *p;

    p = aug_entry_find(path);
    if (p != NULL)
        return p->value;

    return NULL;
}

int aug_set(const char *path, const char *value) {
    struct aug_entry *p;

    p = aug_entry_find(path);
    if (p == NULL) {
        p = aug_entry_make(path, head);
        if (p == NULL)
            return -1;
    }
    if (p->value != NULL) {
        free((char *) p->value);
    }
    p->value = strdup(value);
    if (p->value == NULL)
        return -1;
    return 0;
}

int aug_exists(const char *path) {
    return (aug_entry_find(path) != NULL);
}

int aug_insert(const char *path, const char *sibling) {
    struct aug_entry *s, *p;
    char *pdir, *sdir;

    if (STREQ(path, sibling))
        return -1;

    pdir = strrchr(path, SEP);
    sdir = strrchr(sibling, SEP);
    if (pdir == NULL || sdir == NULL)
        return -1;
    if (pdir - path != sdir - sibling)
        return -1;
    if (STRNEQLEN(path, sibling, pdir - path))
        return -1;

    s = aug_entry_find(sibling);
    if (s == NULL)
        return -1;
    
    p = aug_entry_find(path);
    if (p == NULL) {
        return aug_entry_make(path, s) == NULL ? -1 : 0;
    } else {
        p->prev->next = p->next;
        p->next->prev = p->prev;
        s->prev->next = p;
        p->prev = s->prev;
        p->next = s;
        s->prev = p;
        return 0;
    }
}

int aug_rm(const char *path) {
    struct aug_entry *p;
    int cnt = 0;

    for (p = head->next->next; p != head->next; p = p->next) {
        int plen = pathlen(path);
        if (STREQLEN(p->prev->path, path, pathlen(path)) &&
            (p->prev->path[plen] == '\0' || p->prev->path[plen] == '/') &&
            ! STREQ(P_SYSTEM, p->prev->path) &&
            ! STREQ(P_SYSTEM_CONFIG, p->prev->path)) {
            aug_entry_free(p->prev);
            cnt += 1;
        }
    }
    return cnt;
}

int aug_ls(const char *path, const char ***children) {
    int cnt = 0;
    int len = pathlen(path);
    struct aug_entry *p;

    for (int copy=0; copy <=1; copy++) {
        // Do two passes: the first one to count, the second one
        // to actually copy the children
        if (copy) {
            if (children == NULL)
                return cnt;

            *children = calloc(cnt, sizeof(char *));
            if (*children == NULL)
                return -1;
        }

        p = head;
        cnt = 0;
        do {
            if (STREQLEN(p->path, path, len)
                && strlen(p->path) > len + 1 
                && strchr(p->path + len + 1, SEP) == NULL) {
                if (copy)
                    (*children)[cnt] = p->path;
                cnt += 1;
            }
            p = p->next;
        } while (p != head);
    }
    return cnt;
}

int aug_save(void) {
    return augp_pam.save();
}

void aug_print(FILE *out, const char *path) {
    struct aug_entry *p;

    p = head;
    do {
        if (p != p->prev->next)
            FIXME("Wrong prev->next for %s", p->path);
        if (p != p->next->prev)
            FIXME("Wrong next-> for %s", p->path);
            
        if (path == NULL || STREQLEN(path, p->path, pathlen(path))) {
            fprintf(out, p->path);
            if (p->value != NULL) {
                fprintf(out, " = %s", p->value);
            }
            fputc('\n', out);
        }
        p = p->next;
    } while (p != head);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

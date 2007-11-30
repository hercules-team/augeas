/*
 * internal.c: internal data structures and helpers
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

#include "internal.h"

void aug_token_free(struct aug_token *t) {
    if (t != NULL) {
        safe_free((void *) t->text);
        safe_free((void *) t->node);
        free(t);
    }
}

void aug_file_free(struct aug_file *af) {
    if (af != NULL) {
        safe_free((void *) af->name);
        safe_free((void *) af->node);
        while (af->tokens != NULL) {
            struct aug_token *t = af->tokens;
            af->tokens = t->next;
            free(t);
        }
    }
}

struct aug_token *aug_make_token(enum aug_token_type type,
                                 const char *text,
                                 const char *node) {
    struct aug_token *result;

    result = calloc(1, sizeof(struct aug_token));
    if (result == NULL)
        return NULL;

    result->type = type;
    result->text = text;
    result->node = node;
    return result;
}

struct aug_token *aug_insert_token(struct aug_token *t,
                                   enum aug_token_type type,
                                   const char *text,
                                   const char *node) {
    struct aug_token *result;

    result = aug_make_token(type, text, node);
    result->next = t->next;
    t->next = result;
    return result;
}

struct aug_token *aug_file_append_token(struct aug_file *af,
                                        enum aug_token_type type,
                                        const char *text,
                                        const char *node) {
    struct aug_token *result;

    result = aug_make_token(type, text, node);
    if (result == NULL)
        return NULL;

    if (af->tokens == NULL) {
        af->tokens = result;
    } else {
        struct aug_token *t;
        for (t = af->tokens; t->next != NULL; t = t->next);
        t->next = result;
    }
    return result;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

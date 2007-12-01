/*
 * record.c: Support for parsing fixed-field records
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

#include "record.h"
#include "internal.h"
#include "augeas.h"

#include <pcre.h>

/* Size of the line buffer during parsing */
#define MAX_LINE 256

struct aug_rec_spec {
    pcre                 *re;
    int                  nfields;
    const char           **fields;
    struct aug_rec_spec  *next;
};

struct aug_rec {
    struct aug_rec_spec *specs;
    unsigned int         ovecsize; /* Size needed for the ovector arg 
                                      for pcre_exec */
    const char          *fsep;     /* Field separator for new records */
    const char          *rsep;     /* Record separator */
};

aug_rec_t aug_rec_init(const char *fsep, const char *rsep) {
    aug_rec_t result;

    result = calloc(1, sizeof(struct aug_rec));
    if (result == NULL)
        return NULL;

    result->fsep = strdup(fsep);
    result->rsep = strdup(rsep);
    if (result->fsep == NULL || result->rsep == NULL) {
        aug_rec_free(result);
        return NULL;
    }
    return result;
}

static void aug_rec_spec_free(struct aug_rec_spec *spec) {
    if (spec != NULL) {
        safe_free(spec->re);
        if (spec->fields != NULL) {
            for (int i=0; i < spec->nfields; i++)
                safe_free((void *) spec->fields[i]);
            free(spec->fields);
        }
        free(spec);
    }
}

void aug_rec_free(aug_rec_t rec) {
    if (rec != NULL) {
        safe_free((void *) rec->fsep);
        safe_free((void *) rec->rsep);

        while (rec->specs != NULL) {
            struct aug_rec_spec *spec = rec->specs;
            rec->specs = spec->next;
            aug_rec_spec_free(spec);
        }
    }
}

int aug_rec_add_spec(aug_rec_t rec, const char *pattern, 
                     int nfields, const char **fields) {
    struct aug_rec_spec *spec;
    const char *errptr;
    int erroffset;
    pcre *re;
    int r, ngroups;

    re = pcre_compile(pattern, 0, &errptr, &erroffset, NULL);
    if (re == NULL)
        return -1;

    spec = calloc(1, sizeof(struct aug_rec_spec));
    if (spec == NULL) {
        free(re);
        return -1;
    }
    spec->re = re;

    r = pcre_fullinfo(re, NULL, PCRE_INFO_CAPTURECOUNT, &ngroups);
    if (r != 0) {
        aug_rec_spec_free(spec);
        return -1;
    }
    if (3*(ngroups+1) > rec->ovecsize)
        rec->ovecsize = 3*(ngroups + 1);

    spec->nfields = nfields;
    if (spec->nfields > 0) {
        spec->fields = calloc(nfields, sizeof(char *));
        if (spec->fields == NULL) {
            aug_rec_spec_free(spec);
            return -1;
        }
        for (int i=0; i < nfields; i++) {
            if (fields[i] != NULL) {
                spec->fields[i] = strdup(fields[i]);
                if (spec->fields[i] == NULL) {
                    aug_rec_spec_free(spec);
                    return -1;
                }
            }
        }
    }

    if (rec->specs == NULL) {
        rec->specs = spec;
    } else {
        struct aug_rec_spec *p;
        for (p = rec->specs; p->next != NULL; p = p->next);
        p->next = spec;
    }
    return 0;
}

static char *aug_rec_insert_key(const char *recnode, 
                                const char *name, const char *value) {
    char *node = NULL;
    int r;
    
    r = asprintf(&node, "%s/%s", recnode, name);
    if (r == -1)
        return NULL;
    aug_set(node, value);
    return node;
}

struct aug_file *aug_rec_parse(const aug_rec_t rec, const char *path,
                               const char *prefix) {
    FILE *fp = fopen(path, "r");
    char line[MAX_LINE];
    struct aug_file *result;
    int r, seq = 0;

    if (fp == NULL)
        return NULL;

    result = calloc(1, sizeof(struct aug_file));
    if (result == NULL)
        goto error;
    result->name = strdup(path);
    result->node = strdup(prefix);
    if (result->name == NULL || result->node == NULL)
        goto error;

    while (fgets(line, MAX_LINE, fp) != NULL) {
        struct aug_rec_spec *spec = NULL;
        int matches[rec->ovecsize];
        int nmatch;

        for (spec=rec->specs; spec != NULL; spec = spec->next) {
            nmatch = pcre_exec(spec->re, NULL, line, strlen(line), 0, 0,
                               matches, rec->ovecsize);
            if (nmatch >= 1)
                break;
            /* nmatch == 0 is an error since that means pcre_exec
               ran out of room in matches */
            if (nmatch != PCRE_ERROR_NOMATCH)
                goto error;
        }

        if (spec != NULL && spec->fields != NULL) {
            const char *match;
            char *recnode;
            int inserted = 0;

            r = asprintf(&recnode, "%s/%d", prefix, seq);
            if (r == -1)
                goto error;

            for (int i=0; i < spec->nfields; i++) {
                r = pcre_get_substring(line, matches, nmatch, i+1, &match);
                if (r < 0)
                    goto error;
                if (spec->fields[i][0] == '_') {
                    aug_file_append_token(result, AUG_TOKEN_SEP, match, NULL);
                } else {
                    char *node;
                    node = aug_rec_insert_key(recnode, spec->fields[i], match);
                    if (node == NULL)
                        goto error;
                    aug_file_append_token(result, AUG_TOKEN_VALUE, match, node);
                    inserted = 1;
                }
            }
            aug_file_append_token(result, AUG_TOKEN_EOR, NULL, recnode);
            if (inserted)
                seq += 1;
        } else {
            aug_file_append_token(result, AUG_TOKEN_INERT, strdup(line), NULL);
        }
    }
    aug_file_append_token(result, AUG_TOKEN_EOF, NULL, NULL);
    fclose(fp);
   
    return result;
 error:
    if (fp != NULL)
        fclose(fp);
    if (result != NULL)
        aug_file_free(result);

    return NULL;
}

static int aug_rec_handle_inserts(struct aug_file *af) {
    int nrec;
    int result = 0;
    const char **records = NULL;
    const char **fields = NULL;
    struct aug_token *t;
 
    nrec = aug_ls(af->node, &records);
    if (nrec == -1)
        return -1;

    for (int i=0; i < nrec; i++) {
        for (t=af->tokens; t->type != AUG_TOKEN_EOF; t = t->next) {
            if (t->type == AUG_TOKEN_EOR && STREQ(records[i], t->node))
                break;
        }
        // Is records[i] a new record ?
        if (t->type == AUG_TOKEN_EOF) {
            int nfld;

            result = 1;
            nfld = aug_ls(records[i], &fields);
            if (nfld == -1)
                goto error;
            if (i == 0) {
                t = af->tokens;
            } else {
                for (t=af->tokens; t->type != AUG_TOKEN_EOF; t = t->next) {
                    if (t->type == AUG_TOKEN_EOR 
                        && STREQ(records[i-1], t->node))
                        break;
                }
                if (t->type == AUG_TOKEN_EOF)
                    goto error;
            }
            for (int f=0; f<nfld; f++) {
                if (f>0)
                    t = aug_insert_token(t, AUG_TOKEN_SEP, NULL, NULL);
                t = aug_insert_token(t, AUG_TOKEN_VALUE,
                                     strdup(aug_get(fields[f])),
                                     strdup(fields[f]));
            }
            t = aug_insert_token(t, AUG_TOKEN_EOR, NULL, records[i]);
        }
    }

    free(records);
    return result;
 error:
    safe_free(records);
    safe_free(fields);
    return -1;
}

int aug_rec_save(const aug_rec_t rec, struct aug_file *af) {
    char *name;
    FILE *fp = NULL;
    int dirty = 0;

    if (af == NULL || rec == NULL)
        return -1;

    dirty = aug_rec_handle_inserts(af);
    if (dirty == -1)
        return -1;

    // Check for changes to preexisting records
    for (struct aug_token *t=af->tokens; t != NULL; t = t->next) {
        const char *text;
        if (t->node != NULL && t->type == AUG_TOKEN_VALUE) {
            text = aug_get(t->node);
            if (text != NULL && STRNEQ(text, t->text)) {
                // Field has been changed
                dirty = 1;
                if (strlen(t->text) == 0) {
                    // Field was not in initial input
                    struct aug_token *p;
                    for (p=af->tokens; p->next != t; p = p->next);
                    if (p->type == AUG_TOKEN_SEP && strlen(p->text) == 0) {
                        // Separator wasn't there either
                        free((void *) p->text);
                        p->text = NULL;
                    }
                }
                safe_free((void *) t->text);
                t->text = strdup(text);
                if (t->text == NULL)
                    return -1;
            } else if (text == NULL) {
                // Field was deleted
                dirty = 1;
                safe_free((void *) t->text);
                t->text = NULL;
            }
#if 0
            // Insert missing separators
            if (t->next->type == AUG_TOKEN_VALUE) {
                fprintf(stderr, "III %s\n", t->node);
                dirty = 1;
                struct aug_token *s = aug_make_token(AUG_TOKEN_SEP, NULL, NULL);
                if (s == NULL)
                    goto error;
                s->next = t->next;
                t->next = s;
            }
#endif
        }
    }
    if (! dirty)
        return 0;

    // For now, we save into af->name + ".augnew"
    name = alloca(strlen(af->name) + 1 + strlen(".augnew") + 1);
    sprintf(name, "%s.augnew", af->name);

    fp = fopen(name, "w");
    if (fp == NULL)
        return -1;

    for (struct aug_token *t=af->tokens; t != NULL; t = t->next) {
        switch(t->type) {
        case AUG_TOKEN_INERT:
            fprintf(fp, t->text);
            break;
        case AUG_TOKEN_SEP:
            if (t->text == NULL) {
                fprintf(fp, rec->fsep);
            } else {
                fprintf(fp, t->text);
            }
            break;
        case AUG_TOKEN_VALUE:
            if (t->text != NULL) {
                fprintf(fp, t->text);
            } else {
                while (t->next->type == AUG_TOKEN_SEP) {
                    t = t->next;
                }
            }
            break;
        case AUG_TOKEN_EOR:
            fprintf(fp, t->text == NULL ? rec->rsep : t->text);
            break;
        case AUG_TOKEN_EOF:
            goto done;
        default:
            FIXME("Unexpected token %d saving", t->type);
            break;
        }
    }

 done:
    fclose(fp);
    return 0;
#if 0
 error:
    if (fp != NULL)
        fclose(fp);
    return -1;
#endif
}


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

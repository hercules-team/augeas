/*
 * prov_spec.c: parsed file provider
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
#include "ast.h"
#include "list.h"
#include "config.h"

#include <ftw.h>
#include <glob.h>

/* Arbitrary limit on the number of fd's to use with ftw */
#define MAX_DESCRIPTORS 10

/* Extension for newly created files */
#define EXT_AUGNEW ".augnew"
/* Extension for backup files */
#define EXT_AUGSAVE ".augsave"

int augp_spec_init(struct augeas *);
int augp_spec_load(struct augeas *);
int augp_spec_save(struct augeas *);

struct augp_spec_data {
    struct grammar  *grammars;
    struct map      *maps;
    struct aug_file *files;
    unsigned int     gf_flags;
};

// FIXME: This whole business is unsavory, and not threadsafe
struct augp_spec_data augp_spec_data = {
    .grammars = NULL,
    .maps = NULL
};

const struct aug_provider augp_spec = {
    .name = "spec",
    .init = augp_spec_init,
    .load = augp_spec_load,
    .save = augp_spec_save
};

/* Return true if TREE or any of its children is marked as dirty. The
   dirty flag is reset throughout TREE at the same time */
static int tree_dirty(struct tree *tree) {
    int dirty = tree->dirty;

    tree->dirty = 0;
    list_for_each(c, tree->children) {
        if (tree_dirty(c))
            dirty = 1;
    }
    return dirty;
}

static int ftw_load_cb(const char *fpath, 
                       ATTRIBUTE_UNUSED const struct stat *st,
                       int type) {
    int r;
    struct grammar *grammars;
    struct map *maps;

    if (type != FTW_F)
        return 0;

    if (!STREQ(fpath + strlen(fpath) - 4, ".aug"))
        return 0;

    /* FIXME: fpath may be a socket, fifo etc. but ignore that for now */

    r = load_spec(fpath, stderr, augp_spec_data.gf_flags, &grammars, &maps);
    if (r == -1) {
        /* FIXME: Record the error somewhere, but keep going */
        return 0;
    }
    list_append(augp_spec_data.grammars, grammars);
    list_append(augp_spec_data.maps, maps);

    return 0;
}

/* Read spec files and parse maps/grammars into augp_spec_data. Spec files
 * are read from AUGEAS_LENS_DIR and from any directory mentioned on the
 * env var AUGEAS_LENS_LIB
 */
int augp_spec_init(struct augeas *aug) {
    int r;
    char *env, *path, *p;

    augp_spec_data.gf_flags = GF_NONE;
    if (aug->flags & AUG_TYPE_CHECK)
        augp_spec_data.gf_flags |= GF_LENS_TYPE_CHECK;

    r = ftw(AUGEAS_LENS_DIR, ftw_load_cb, MAX_DESCRIPTORS);
    if (r == -1) {
        if (errno != EACCES && errno != ENOENT) {
            fprintf(stderr, "Ignoring failure of walk of %s.\n"
                    "  Reason was: %s\n",
                    AUGEAS_LENS_DIR, strerror(errno));
        }
    }

    env = getenv(AUGEAS_LENS_ENV);
    if (env != NULL) {
        env = strndup(env, MAX_ENV_SIZE);
        path = env;
        do {
            for (p = path; *p != '\0' && *p != PATH_SEP_CHAR; p++);

            if (*p == PATH_SEP_CHAR)
                *p++ = '\0';

            r = ftw(path, ftw_load_cb, MAX_DESCRIPTORS);
            if (r == -1) {
                if (errno != EACCES && errno != ENOENT) {
                    fprintf(stderr, "Ignoring failure of walk of %s.\n"
                            "  Reason was: %s\n",
                            path, strerror(errno));
                }
            }
            path = p;
        } while (*path != '\0');
    }

    // CHECK: Multiple grammars with the same name
    list_for_each(map, augp_spec_data.maps) {
        list_for_each(g, augp_spec_data.grammars) {
            if (STREQ(map->grammar_name, g->name))
                map->grammar = g;
        }
    }

    /* Maps that reference no grammar are removed */
    int changed;
    do {
        changed = 0;
        list_for_each(map, augp_spec_data.maps) {
            if (map->grammar == NULL) {
                grammar_error(map->filename, map->lineno,
                              "grammar %s not loaded. Ignoring map.", 
                              map->grammar_name);
                list_remove(map, augp_spec_data.maps);
                augs_map_free(map);
                changed = 1;
                break;
            }
        }
    } while (changed);
    return 0;
}

static char *pathjoin(char *path, const char *seg) {
    int len = strlen(seg) + 1;

    if (path != NULL) {
        len += strlen(path) + 1;
        path = realloc(path, len);
        if (strlen(path) == 0 || path[strlen(path)-1] != SEP)
            strcat(path, "/");
        if (seg[0] == SEP)
            seg += 1;
        strcat(path, seg);
    } else {
        path = malloc(len);
        strcpy(path, seg);
    }
    return path;
}

static const char *file_node(const char *filename, struct filter *filter) {
    char *result = NULL;

    list_for_each(e, filter->path) {
        const char *seg = NULL;
        if (e->type == E_CONST) {
            seg = e->text;
        } else if (e->type == E_GLOBAL && STREQ(e->text, "basename")) {
            seg = strrchr(filename, SEP);
            if (seg == NULL)
                seg = filename;
            else
                seg += 1;
        } else {
            internal_error(NULL, filter->lineno,
                      "Illegal path entry for filter %s", filter->glob);
        }
        if (seg != NULL) {
            result = pathjoin(result, seg);
        }
    }
    return result;
}

static char *add_load_info(struct augeas *aug, const char *filename,
                           const char *node,
                           const char *grammarname) {
    static const char *const metatree = "/augeas/files";
    static const char *const pnode = "/path";
    static const char *const gnode = "/grammar";
    static const char *const enode = "/error";

    char *result = NULL;
    int end = 0;
    result = pathjoin(result, metatree);
    result = pathjoin(result, filename + strlen(aug->root));
    end = strlen(result);

    result = pathjoin(result, pnode);
    aug_set(aug, result, node);
    result[end] = '\0';

    result = pathjoin(result, gnode);
    aug_set(aug, result, grammarname);
    result[end] = '\0';

    result = pathjoin(result, enode);
    return result;
}

static int parse_file(struct augeas *aug,
                      const char *filename, struct filter *filter,
                      struct grammar *grammar) {
    const char *text = NULL;
    const char *node = file_node(filename, filter);
    char *err_node = NULL;
    const char *err_status = NULL;
    struct aug_file *file = NULL;
    struct tree *tree;

    err_node = add_load_info(aug, filename, node, grammar->name);
    if (err_node == NULL)
        goto error;

    text = aug_read_file(filename);
    if (text == NULL) {
        err_status = "read_failed";
        goto error;
    }

    file = aug_make_file(filename, node, grammar);
    if (file == NULL) {
        err_status = "out_of_memory";
        goto error;
    }
    free((void *) node);
    node = NULL;

    tree = parse(file, text, stdout, GF_NONE);
    if (tree == NULL) {
        err_status = "parse_failed";
        goto error;
    }

    list_append(augp_spec_data.files, file);
    free((void *) text);

    aug_tree_replace(aug, file->node, tree);
    tree_dirty(tree);

    aug_set(aug, err_node, NULL);
    free(err_node);
    return 0;
 error:
    if (err_node != NULL)
        aug_set(aug, err_node, err_status);
    free(err_node);
    free(file);
    free((void *) text);
    return -1;
}

static int strendswith(const char *s, const char *end) {
    const char *p = s + strlen(s) - strlen(end);
    return STREQ(p, end);
}

/* Parse all the files mentioned in maps and load them into the tree */
int augp_spec_load(struct augeas *aug) {
    glob_t globbuf;
    int ret = 0;
    int flags = GLOB_NOSORT;

    list_for_each(map, augp_spec_data.maps) {
        list_for_each(filter, map->filters) {
            char *globpat;

            ret = asprintf(&globpat, "%s%s", aug->root,
                           filter->glob);
            if (ret == -1)
                goto exit;
            ret = glob(globpat, flags, NULL, &globbuf);
            free(globpat);

            if (ret != 0 && ret != GLOB_NOMATCH) {
                ret = -1;
                goto exit;
            }

            for (int i=0; i < globbuf.gl_pathc; i++) {
                const char *s = globbuf.gl_pathv[i];
                if (strendswith(s, ".augnew") || strendswith(s, ".augnew.dot"))
                    continue;
                parse_file(aug, globbuf.gl_pathv[i], filter, map->grammar);
            }
        }
    }

 exit:
    /* If we called glob at least once, free globbuf */
    if (flags & GLOB_APPEND)
        globfree(&globbuf);
    return ret;
}

int augp_spec_save(struct augeas *aug) {
    char *augnew = calloc(1, 80);  /* Completely arbitrary initial size */
    char *augsave = calloc(1, 80);
    int ret = -1;
    
    list_for_each(file, augp_spec_data.files) {
        // FIXME: If the whole tree went away, we should probably delete
        // the file; but we don't have a mechanism to create a file, so
        // we just leave an empty file there
        struct tree *tree = aug_tree_find(aug->tree, file->node);

        if (tree == NULL || tree_dirty(tree)) {
            FILE *fp;
            augnew = realloc(augnew, 
                             strlen(file->name) + strlen(EXT_AUGNEW) + 1);
            sprintf(augnew, "%s" EXT_AUGNEW, file->name);
            
            fp = fopen(augnew, "w");
            if (fp == NULL)
                goto done;

            if (tree != NULL)
                put(fp, tree->children, file);

            if (fclose(fp) != 0)
                goto done;
            if (!(aug->flags & AUG_SAVE_NEWFILE)) {
                if (aug->flags & AUG_SAVE_BACKUP) {
                    augsave = realloc(augsave,
                               strlen(file->name) + strlen(EXT_AUGSAVE) + 1);
                    sprintf(augsave, "%s" EXT_AUGSAVE, file->name);
                    if (rename(file->name, augsave) != 0)
                        goto done;
                }
                if (rename(augnew, file->name) != 0)
                    goto done;
            }
        }
    }
    ret = 0;
 done:
    free(augnew);
    free(augsave);
    return ret;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

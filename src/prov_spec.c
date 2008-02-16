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

int augp_spec_init(void);
int augp_spec_load(void);
int augp_spec_save(void);

struct augp_spec_data {
    struct grammar  *grammars;
    struct map      *maps;
    struct aug_file *files;
    const  char     *root;     // The root in the filesystem
};

struct augp_spec_data augp_spec_data = {
    .grammars = NULL,
    .maps = NULL,
    .root = "/"
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

    printf("Load %s\n", fpath);
    r = load_spec(fpath, stderr, 0, &grammars, &maps);
    if (r == -1) {
        /* FIXME: Record the error somewhere, but keep going */
        return 0;
    }
    list_append(augp_spec_data.grammars, grammars);
    list_append(augp_spec_data.maps, maps);

    return 0;
}

/* Read spec files and parse maps/grammars into augp_spec_data. Spec files
 * are read from AUGEAS_SPEC_DIR (usually /usr/share/augeas/spec) and from
 * any directory mentioned on the env var AUGEAS_SPECLIB
 */
int augp_spec_init(void) {
    int r;
    char *env, *path, *p;
    
    r = ftw(AUGEAS_SPEC_DIR, ftw_load_cb, MAX_DESCRIPTORS);
    if (r == -1) {
        if (errno != EACCES && errno != ENOENT) {
            fprintf(stderr, "Ignoring failure of walk of %s.\n"
                    "  Reason was: %s\n",
                    AUGEAS_SPEC_DIR, strerror(errno));
        }
    }
    
    /* We report the root dir in AUGEAS_META_ROOT, but we only use the
       value we store internally, to avoid any problems with
       AUGEAS_META_ROOT getting changed. To make the tree entry the
       canonical location of the root dir, we'd need some real security on
       who is allowed to change tree entries */
    env = getenv(AUGEAS_ROOT_ENV);
    if (env != NULL) {
        augp_spec_data.root = env;
    }
    aug_set(AUGEAS_META_ROOT, augp_spec_data.root);
    
    env = getenv(AUGEAS_SPEC_ENV);
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

static char *add_load_info(const char *filename, const char *node,
                           const char *grammarname) {
    static const char *const metatree = "/augeas/files";
    static const char *const pnode = "/path";
    static const char *const gnode = "/grammar";
    static const char *const enode = "/error";

    char *result = NULL;
    int end = 0;
    result = pathjoin(result, metatree);
    result = pathjoin(result, filename + strlen(augp_spec_data.root));
    end = strlen(result);
    
    result = pathjoin(result, pnode);
    aug_set(result, node);
    result[end] = '\0';
    
    result = pathjoin(result, gnode);
    aug_set(result, grammarname);
    result[end] = '\0';

    result = pathjoin(result, enode);
    return result;
}

static int parse_file(const char *filename, struct filter *filter, 
                      struct grammar *grammar) {
    const char *text = NULL;
    const char *node = file_node(filename, filter);
    char *err_node = NULL;
    const char *err_status = NULL;
    struct aug_file *file;
    struct tree *tree;

    err_node = add_load_info(filename, node, grammar->name);
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

    tree = parse(file, text, stdout, 0);
    if (tree == NULL) {
        err_status = "parse_failed";
        goto error;
    }

    list_append(augp_spec_data.files, file);
    free((void *) text);

    aug_tree_replace(file->node, tree);
    tree_dirty(tree);

    aug_set(err_node, NULL);
    free(err_node);
    return 0;
 error:
    if (err_node != NULL)
        aug_set(err_node, err_status);
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
int augp_spec_load(void) {
    glob_t globbuf;
    int ret = 0;
    int flags = GLOB_NOSORT;

    list_for_each(map, augp_spec_data.maps) {
        list_for_each(filter, map->filters) {
            char *globpat;

            ret = asprintf(&globpat, "%s%s", augp_spec_data.root, 
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
                parse_file(globbuf.gl_pathv[i], filter, map->grammar);
            }
        }
    }

 exit:
    /* If we called glob at least once, free globbuf */
    if (flags & GLOB_APPEND)
        globfree(&globbuf);
    return -1;
}

int augp_spec_save(void) {
    char *name = calloc(1, 80); /* Completely arbitrary initial size */
    list_for_each(file, augp_spec_data.files) {
        FILE *fp;
        // For now, we save into af->name + ".augnew"
        name = realloc(name, strlen(file->name) + 1 
                       + strlen(".augnew.dot") + 1);

        sprintf(name, "%s.augnew", file->name);

        // FIXME: If the whole tree went away, we should probably delete
        // the file; but we don't have a mechanism to create a file, so
        // we just leave an empty file there
        struct tree *tree = aug_tree_find(aug_tree, file->node);

        if (tree == NULL || tree_dirty(tree)) {
            fp = fopen(name, "w");
            if (fp == NULL) {
                free(name);
                return -1;
            }

            if (tree != NULL)
                put(fp, tree->children, file);
            fclose(fp);
        }
    }
    free(name);
    return 0;
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

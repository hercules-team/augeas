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
};

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
        if (strlen(path) > 0 && path[strlen(path)-1] != SEP)
            strcat(path, "/");
        strcat(path, seg);
    } else {
        path = malloc(len);
        strcpy(path, seg);
    }
    return path;
}

static const char *file_root(const char *filename, struct filter *filter) {
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

static int parse_file(const char *filename, struct filter *filter, 
                      struct grammar *grammar) {
    const char *text = aug_read_file(filename);
    const char *root = file_root(filename, filter);
    struct aug_file *file;
    struct tree *tree;

    printf("Parse %s to %s with %s\n", filename, root, grammar->name);

    if (text == NULL) {
        fprintf(stderr, "Failed to read %s\n", filename);
        return -1;
    }

    file = aug_make_file(filename, root, grammar);
    if (file == NULL)
        goto error;
    free((void *) root);
    root = NULL;

    tree = parse(file, text, stdout, 0);
    if (tree == NULL) {
        fprintf(stderr, "Parsing of %s failed\n", filename);
        goto error;
    }

    list_append(augp_spec_data.files, file);
    free((void *) text);

    aug_tree_replace(file->node, tree);
    tree_dirty(tree);

    return 0;
 error:
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
    const char *root = getenv(AUGEAS_ROOT_ENV);

    list_for_each(map, augp_spec_data.maps) {
        list_for_each(filter, map->filters) {
            char *globpat;
            if (root == NULL) {
                globpat = (char *) filter->glob;
            } else {
                ret = asprintf(&globpat, "%s%s", root, filter->glob);
                if (ret == -1)
                    goto exit;
            }
            ret = glob(globpat, flags, NULL, &globbuf);
            if (root != NULL)
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

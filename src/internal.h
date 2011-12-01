/*
 * internal.h: Useful definitions
 *
 * Copyright (C) 2007-2011 David Lutterkort
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

#ifndef INTERNAL_H_
#define INTERNAL_H_

#include "list.h"
#include "datadir.h"
#include "augeas.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <locale.h>

/*
 * Various parameters about env vars, special tree nodes etc.
 */

/* Define: AUGEAS_LENS_DIR
 * The default location for lens definitions */
#define AUGEAS_LENS_DIR DATADIR "/augeas/lenses"

/* The directory where we install lenses distribute with Augeas */
#define AUGEAS_LENS_DIST_DIR DATADIR "/augeas/lenses/dist"

/* Define: AUGEAS_ROOT_ENV
 * The env var that points to the chroot holding files we may modify.
 * Mostly useful for testing */
#define AUGEAS_ROOT_ENV "AUGEAS_ROOT"

/* Define: AUGEAS_FILES_TREE
 * The root for actual file contents */
#define AUGEAS_FILES_TREE "/files"

/* Define: AUGEAS_META_TREE
 * Augeas reports some information in this subtree */
#define AUGEAS_META_TREE "/augeas"

/* Define: AUGEAS_META_FILES
 * Information about files */
#define AUGEAS_META_FILES AUGEAS_META_TREE AUGEAS_FILES_TREE

/* Define: AUGEAS_META_ROOT
 * The root directory */
#define AUGEAS_META_ROOT AUGEAS_META_TREE "/root"

/* Define: AUGEAS_META_SAVE_MODE
 * How we save files. One of 'backup', 'overwrite' or 'newfile' */
#define AUGEAS_META_SAVE_MODE AUGEAS_META_TREE "/save"

/* Define: AUGEAS_CLONE_IF_RENAME_FAILS
 * Control what save does when renaming the temporary file to its final
 * destination fails with EXDEV or EBUSY: when this tree node exists, copy
 * the file contents. If it is not present, simply give up and report an
 * error.  */
#define AUGEAS_COPY_IF_RENAME_FAILS \
    AUGEAS_META_SAVE_MODE "/copy_if_rename_fails"

/* Define: AUGEAS_CONTEXT
 * Context prepended to all non-absolute paths */
#define AUGEAS_CONTEXT AUGEAS_META_TREE "/context"

/* A hierarchy where we record certain 'events', e.g. which tree
 * nodes actually gotsaved into files */
#define AUGEAS_EVENTS AUGEAS_META_TREE "/events"

#define AUGEAS_EVENTS_SAVED AUGEAS_EVENTS "/saved"

/* Where to put information about parsing of path expressions */
#define AUGEAS_META_PATHX AUGEAS_META_TREE "/pathx"

/* Define: AUGEAS_SPAN_OPTION
 * Enable or disable node indexes */
#define AUGEAS_SPAN_OPTION AUGEAS_META_TREE "/span"

/* Define: AUGEAS_LENS_ENV
 * Name of env var that contains list of paths to search for additional
   spec files */
#define AUGEAS_LENS_ENV "AUGEAS_LENS_LIB"

/* Define: MAX_ENV_SIZE
 * Fairly arbitrary bound on the length of the path we
 *  accept from AUGEAS_SPEC_ENV */
#define MAX_ENV_SIZE 4096

/* Define: PATH_SEP_CHAR
 * Character separating paths in a list of paths */
#define PATH_SEP_CHAR ':'

/* Constants for setting the save mode via the augeas path at
 * AUGEAS_META_SAVE_MODE */
#define AUG_SAVE_BACKUP_TEXT "backup"
#define AUG_SAVE_NEWFILE_TEXT "newfile"
#define AUG_SAVE_NOOP_TEXT "noop"
#define AUG_SAVE_OVERWRITE_TEXT "overwrite"

/* constants for options in the tree */
#define AUG_ENABLE "enable"
#define AUG_DISABLE "disable"

/* default value for the relative path context */
#define AUG_CONTEXT_DEFAULT "/files"

#ifdef __GNUC__

#ifndef __GNUC_PREREQ
#define __GNUC_PREREQ(maj,min) 0
#endif

/**
 * ATTRIBUTE_UNUSED:
 *
 * Macro to flag conciously unused parameters to functions
 */
#ifndef ATTRIBUTE_UNUSED
#define ATTRIBUTE_UNUSED __attribute__((__unused__))
#endif

/**
 * ATTRIBUTE_FORMAT
 *
 * Macro used to check printf/scanf-like functions, if compiling
 * with gcc.
 */
#ifndef ATTRIBUTE_FORMAT
#define ATTRIBUTE_FORMAT(args...) __attribute__((__format__ (args)))
#endif

#ifndef ATTRIBUTE_PURE
#define ATTRIBUTE_PURE __attribute__((pure))
#endif

#ifndef ATTRIBUTE_RETURN_CHECK
#if __GNUC_PREREQ (3, 4)
#define ATTRIBUTE_RETURN_CHECK __attribute__((__warn_unused_result__))
#else
#define ATTRIBUTE_RETURN_CHECK
#endif
#endif

#else
#define ATTRIBUTE_UNUSED
#define ATTRIBUTE_FORMAT(...)
#define ATTRIBUTE_PURE
#define ATTRIBUTE_RETURN_CHECK
#endif                                   /* __GNUC__ */

#define ARRAY_CARDINALITY(array) (sizeof (array) / sizeof *(array))

/* String equality tests, suggested by Jim Meyering. */
#define STREQ(a,b) (strcmp((a),(b)) == 0)
#define STRCASEEQ(a,b) (strcasecmp((a),(b)) == 0)
#define STRCASEEQLEN(a,b,n) (strncasecmp((a),(b),(n)) == 0)
#define STRNEQ(a,b) (strcmp((a),(b)) != 0)
#define STRCASENEQ(a,b) (strcasecmp((a),(b)) != 0)
#define STREQLEN(a,b,n) (strncmp((a),(b),(n)) == 0)
#define STRNEQLEN(a,b,n) (strncmp((a),(b),(n)) != 0)

ATTRIBUTE_PURE
static inline int streqv(const char *a, const char *b) {
    if (a == NULL || b == NULL)
        return a == b;
    return STREQ(a,b);
}

/* Path length and comparison */

#define SEP '/'

/* Length of PATH without any trailing '/' */
ATTRIBUTE_PURE
static inline int pathlen(const char *path) {
    int len = strlen(path);

    if (len > 0 && path[len-1] == SEP)
        len--;

    return len;
}

/* Return 1 if P1 is a prefix of P2. P1 as a string must have length <= P2 */
ATTRIBUTE_PURE
static inline int pathprefix(const char *p1, const char *p2) {
    if (p1 == NULL || p2 == NULL)
        return 0;
    int l1 = pathlen(p1);

    return STREQLEN(p1, p2, l1) && (p2[l1] == '\0' || p2[l1] == SEP);
}

static inline int pathendswith(const char *path, const char *basenam) {
    const char *p = strrchr(path, SEP);
    if (p == NULL)
        return 0;
    return streqv(p+1, basenam);
}

/* Join NSEG path components (passed as const char *) into one PATH.
   Allocate as needed. Return 0 on success, -1 on failure */
int pathjoin(char **path, int nseg, ...);

/* Call calloc to allocate an array of N instances of *VAR */
#define CALLOC(Var,N) do { (Var) = calloc ((N), sizeof (*(Var))); } while (0)

#define MEMZERO(ptr, n) memset((ptr), 0, (n) * sizeof(*(ptr)));

#define MEMCPY(dest, src, n) memcpy((dest), (src), (n) * sizeof(*(src)))

#define MEMMOVE(dest, src, n) memmove((dest), (src), (n) * sizeof(*(src)))

/**
 * TODO:
 *
 * macro to flag unimplemented blocks
 */
#define TODO 								\
    fprintf(stderr, "%s:%d Unimplemented block\n",			\
            __FILE__, __LINE__);

#define FIXME(msg, args ...)                            \
    do {                                                \
        fprintf(stderr, "%s:%d Fixme: ",                \
                __FILE__, __LINE__);                    \
      fprintf(stderr, msg, ## args);                    \
      fputc('\n', stderr);                              \
    } while(0)

/*
 * Internal data structures
 */

// internal.c

/* Function: escape
 * Escape nonprintable characters within TEXT, similar to how it's done in
 * C string literals. Caller must free the returned string.
 */
char *escape(const char *text, int cnt, const char *extra);

/* Function: unescape */
char *unescape(const char *s, int len, const char *extra);

/* Extra characters to be escaped in strings and regexps respectively */
#define STR_ESCAPES "\"\\"
#define RX_ESCAPES  "/\\"

/* Function: print_chars */
int print_chars(FILE *out, const char *text, int cnt);

/* Function: print_pos
 * Print a pretty representation of being at position POS within TEXT */
void print_pos(FILE *out, const char *text, int pos);
char *format_pos(const char *text, int pos);

/* Function: xread_file
 * Read the contents of file PATH and return them as one long string. The
 * caller must free the result. Return NULL if any error occurs.
 */
char* xread_file(const char *path);

/* Get the error message for ERRNUM in a threadsafe way. Based on libvirt's
 * virStrError
 */
const char *xstrerror(int errnum, char *buf, size_t len);

/* Like asprintf, but set *STRP to NULL on error */
int xasprintf(char **strp, const char *format, ...);

/* Convert S to RESULT with error checking */
int xstrtoint64(char const *s, int base, int64_t *result);

/* Calculate line and column number of character POS in TEXT */
void calc_line_ofs(const char *text, size_t pos, size_t *line, size_t *ofs);

/* Cleans path from user, removing trailing slashes and whitespace */
char *cleanpath(char *path);

/* Take the first LEN characters from the regexp *U and expand any
 * character ranges in it. The expanded regexp, if expansion is necessary,
 * is in U, and the old string is freed. If expansion is not needed or an
 * error happens, U will be unchanged.
 *
 * Return 0 if expansion is not necessary, -1 if an error occurs, and 1 if
 * expansion was needed.
 */
int regexp_c_locale(char **u, size_t *len);

/* Struct: augeas
 * The data structure representing a connection to Augeas. */
struct augeas {
    struct tree      *origin;     /* Actual tree root is origin->children */
    const char       *root;       /* Filesystem root for all files */
                                  /* always ends with '/' */
    unsigned int      flags;      /* Flags passed to AUG_INIT */
    struct module    *modules;    /* Loaded modules */
    size_t            nmodpath;
    char             *modpathz;   /* The search path for modules as a
                                     glibc argz vector */
    struct pathx_symtab *symtab;
    struct error        *error;
    uint                api_entries;  /* Number of entries through a public
                                       * API, 0 when called from outside */
#if HAVE_USELOCALE
    /* On systems that have a uselocale call, we switch to the C locale
     * on entry into API functions, and back to the old user locale
     * on exit.
     * FIXME: We need some solution for systems without uselocale, like
     * setlocale + critical section, though that is very heavy-handed
     */
    locale_t            c_locale;
    locale_t            user_locale;
#endif
};

static inline struct error *err_of_aug(const struct augeas *aug) {
    return ((struct augeas *) aug)->error;
}

/* Used by augparse for loading tests */
int __aug_load_module_file(struct augeas *aug, const char *filename);

/* Called at beginning and end of every _public_ API function */
void api_entry(const struct augeas *aug);
void api_exit(const struct augeas *aug);

/* Struct: tree
 * An entry in the global config tree. The data structure allows associating
 * values with interior nodes, but the API currently marks that as an error.
 *
 * To make dealing with parents uniform, even for the root, we create
 * standalone trees with a fake root, called origin. That root is generally
 * not referenced from anywhere. Standalone trees should be created with
 * MAKE_TREE_ORIGIN.
 *
 * The DIRTY flag is used to track which parts of the tree might need to be
 * saved. For any node that is marked dirty, all of its ancestors must be
 * marked dirty, too. Instead of setting this flag directly, the function
 * TREE_MARK_DIRTY in augeas.c should be used (and only functions in that
 * file should have a need to mark nodes as dirty)
 */
struct tree {
    struct tree *next;
    struct tree *parent;     /* Points to self for root */
    char        *label;      /* Last component of PATH */
    struct tree *children;   /* List of children through NEXT */
    char        *value;
    int          dirty;
    struct span *span;
};

/* The opaque structure used to represent path expressions. API's
 * using STRUCT PATHX are declared farther below
 */
struct pathx;

#define ROOT_P(t) ((t) != NULL && (t)->parent == (t)->parent->parent)

/* Function: make_tree
 * Allocate a new tree node with the given LABEL, VALUE, and CHILDREN,
 * which are not copied. The new tree is marked as dirty
 */
struct tree *make_tree(char *label, char *value,
                       struct tree *parent, struct tree *children);

/* Mark a tree as a standalone tree; this creates a fake parent for ROOT,
 * so that even ROOT has a parent. A new node with only child ROOT is
 * returned on success, and NULL on failure.
 */
struct tree  *make_tree_origin(struct tree *root);

int tree_replace(struct augeas *aug, const char *path, struct tree *sub);
/* Make a new tree node and append it to parent's children */
struct tree *tree_append(struct tree *parent, char *label, char *value);

int tree_rm(struct pathx *p);
int tree_unlink(struct tree *tree);
struct tree *tree_set(struct pathx *p, const char *value);
int tree_insert(struct pathx *p, const char *label, int before);
int free_tree(struct tree *tree);
int dump_tree(FILE *out, struct tree *tree);
int tree_equal(const struct tree *t1, const struct tree *t2);
char *path_expand(struct tree *tree, const char *ppath);
char *path_of_tree(struct tree *tree);
/* Clear the dirty flag in the whole TREE */
void tree_clean(struct tree *tree);
/* Return first child with label LABEL or NULL */
struct tree *tree_child(struct tree *tree, const char *label);
/* Return first existing child with label LABEL or create one. Return NULL
 * when allocation fails */
struct tree *tree_child_cr(struct tree *tree, const char *label);
/* Create a path in the tree; nodes along the path are looked up with
 * tree_child_cr */
struct tree *tree_path_cr(struct tree *tree, int n, ...);
/* Store VALUE directly as the value of TREE and set VALUE to NULL.
 * Update dirty flags */
void tree_store_value(struct tree *tree, char **value);
/* Set the value of TREE to a copy of VALUE and update dirty flags */
int tree_set_value(struct tree *tree, const char *value);
/* Cleanly remove all children of TREE, but leave TREE itself unchanged */
void tree_unlink_children(struct augeas *aug, struct tree *tree);
/* Find the node matching PATH.
 * Returns the node or NULL on error
 * Errors: EMMATCH - more than one node matches PATH
 *         ENOMEM  - allocation error
 */
struct tree *tree_find(struct augeas *aug, const char *path);
/* Find the node matching PATH. Expand the tree to contain such a node if
 * none exists.
 * Returns the node or NULL on error
 */
struct tree *tree_find_cr(struct augeas *aug, const char *path);
/* Find the node at the path stored in AUGEAS_CONTEXT, i.e. the root context
 * node for relative paths.
 * Errors: EMMATCH - more than one node matches PATH
 *         ENOMEM  - allocation error
 */
struct tree *tree_root_ctx(const struct augeas *aug);

/* Struct: memstream
 * Wrappers to simulate OPEN_MEMSTREAM where that's not available. The
 * STREAM member is opened by INIT_MEMSTREAM and closed by
 * CLOSE_MEMSTREAM. The BUF is allocated automatically, but can not be used
 * until after CLOSE_MEMSTREAM has been called. It is the callers
 * responsibility to free up BUF.
 */
struct memstream {
    FILE   *stream;
    char   *buf;
    size_t size;
};

/* Function: init_memstream
 * Initialize a memstream. On systems that have OPEN_MEMSTREAM, it is used
 * to open MS->STREAM. On systems without OPEN_MEMSTREAM, MS->STREAM is
 * backed by a temporary file.
 *
 * MS must be allocated in advance; INIT_MEMSTREAM initializes it.
 */
int __aug_init_memstream(struct memstream *ms);
#define init_memstream(ms) __aug_init_memstream(ms);

/* Function: close_memstream
 * Close a memstream. After calling this, MS->STREAM can not be used
 * anymore and a string representing whatever was written to it is
 * available in MS->BUF. The caller must free MS->BUF.
 *
 * The caller must free the MEMSTREAM structure.
 */
int __aug_close_memstream(struct memstream *ms);
#define close_memstream(ms) __aug_close_memstream(ms)

/*
 * Path expressions
 */

typedef enum {
    PATHX_NOERROR = 0,
    PATHX_ENAME,
    PATHX_ESTRING,
    PATHX_ENUMBER,
    PATHX_EDELIM,
    PATHX_ENOEQUAL,
    PATHX_ENOMEM,
    PATHX_EPRED,
    PATHX_EPAREN,
    PATHX_ESLASH,
    PATHX_EINTERNAL,
    PATHX_ETYPE,
    PATHX_ENOVAR,
    PATHX_EEND,
    PATHX_ENOMATCH,
    PATHX_EARITY,
    PATHX_EREGEXP,
    PATHX_EMMATCH
} pathx_errcode_t;

struct pathx;
struct pathx_symtab;

const char *pathx_error(struct pathx *pathx, const char **txt, int *pos);

/* Parse a path expression PATH rooted at TREE, which is a node somewhere
 * in AUG->ORIGIN. If TREE is NULL, AUG->ORIGIN is used. If ROOT_CTX is not
 * NULL and the PATH isn't absolute then it will be rooted at ROOT_CTX.
 *
 * Use this function rather than PATHX_PARSE for path expressions inside
 * the tree in AUG->ORIGIN.
 *
 * If NEED_NODESET is true, the resulting path expression must evaluate toa
 * nodeset, otherwise it can evaluate to a value of any type.
 *
 * Return the resulting path expression, or NULL on error. If an error
 * occurs, the error struct in AUG contains details.
 */
struct pathx *pathx_aug_parse(const struct augeas *aug,
                              struct tree *tree,
                              struct tree *root_ctx,
                              const char *path, bool need_nodeset);

/* Parse the string PATH into a path expression PX that will be evaluated
 * against the tree ORIGIN.
 *
 * If NEED_NODESET is true, the resulting path expression must evaluate toa
 * nodeset, otherwise it can evaluate to a value of any type.
 *
 * Returns 0 on success, and -1 on error
 */
int pathx_parse(const struct tree *origin,
                struct error *err,
                const char *path,
                bool need_nodeset,
                struct pathx_symtab *symtab,
                struct tree *root_ctx,
                struct pathx **px);
/* Return the error struct that was passed into pathx_parse */
struct error *err_of_pathx(struct pathx *px);
struct tree *pathx_first(struct pathx *path);
struct tree *pathx_next(struct pathx *path);
/* Return -1 if evalutating PATH runs into trouble, otherwise return the
 * number of nodes matching PATH and set MATCH to the first matching
 * node */
int pathx_find_one(struct pathx *path, struct tree **match);
int pathx_expand_tree(struct pathx *path, struct tree **tree);
void free_pathx(struct pathx *path);

struct pathx_symtab *pathx_get_symtab(struct pathx *pathx);
int pathx_symtab_define(struct pathx_symtab **symtab,
                        const char *name, struct pathx *px);
/* Returns 1 on success, and -1 when out of memory */
int pathx_symtab_assign_tree(struct pathx_symtab **symtab, const char *name,
                             struct tree *tree);
int pathx_symtab_undefine(struct pathx_symtab **symtab, const char *name);
void pathx_symtab_remove_descendants(struct pathx_symtab *symtab,
                                     const struct tree *tree);
void free_symtab(struct pathx_symtab *symtab);

/* Debug helpers, all defined in internal.c. When ENABLE_DEBUG is not
 * set, they compile to nothing.
 */
#  if ENABLE_DEBUG
  /* Return true if debugging for CATEGORY is turned on */
  bool debugging(const char *category);
  /* Format the arguments into a file name, prepend it with the directory
   * from the environment variable AUGEAS_DEBUG_DIR, and open the file for
   * writing.
  */
  FILE *debug_fopen(const char *format, ...)
    ATTRIBUTE_FORMAT(printf, 1, 2);
#  else
#    define debugging(facility) (0)
#    define debug_fopen(format ...) (NULL)
#  endif
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

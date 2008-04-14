/*
 * internal.h: Useful definitions
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

#ifndef __INTERNAL_H
#define __INTERNAL_H

#define DEBUG

#include "list.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#ifdef __GNUC__
#ifdef HAVE_ANSIDECL_H
#include <ansidecl.h>
#endif

/* String equality tests, suggested by Jim Meyering. */
#define STREQ(a,b) (strcmp((a),(b)) == 0)
#define STRCASEEQ(a,b) (strcasecmp((a),(b)) == 0)
#define STRNEQ(a,b) (strcmp((a),(b)) != 0)
#define STRCASENEQ(a,b) (strcasecmp((a),(b)) != 0)
#define STREQLEN(a,b,n) (strncmp((a),(b),(n)) == 0)
#define STRNEQLEN(a,b,n) (strncmp((a),(b),(n)) != 0)

__attribute__((pure))
static inline int streqv(const char *a, const char *b) {
    if (a == NULL || b == NULL)
        return a == b;
    return STREQ(a,b);
}

/* Path length and comparison */

#define SEP '/'

/* Length of PATH without any trailing '/' */
__attribute__((pure))
static inline int pathlen(const char *path) {
    int len = strlen(path);

    if (len > 0 && path[len-1] == SEP)
        len--;

    return len;
}

/* Return 1 if P1 is a prefix of P2. P1 as a string must have length <= P2 */
__attribute__((pure))
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

#define REALLOC(var, n) do {                                            \
        (var) = realloc((var), (n) * sizeof (*(var)));                  \
    } while (0)

#define MEMZERO(ptr, n) memset((ptr), 0, (n) * sizeof(*(ptr)));

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

#else
#define ATTRIBUTE_UNUSED
#define ATTRIBUTE_FORMAT(...)
#endif

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

/* Escape nonprintable characters within TEXT, similar to how it's done in
 * C string literals. Caller must free the returned string.
 */
char *escape(const char *text, int cnt);
char *unescape(const char *s, int len);
int print_chars(FILE *out, const char *text, int cnt);

/* Print a pretty representation of being at position POS within TEXT */
void print_pos(FILE *out, const char *text, int pos);
char *format_pos(const char *text, int pos);

/* Read the contents of file PATH and return them as one long string. The
 * caller must free the result. Return NULL if any error occurs.
 */
const char* aug_read_file(const char *path);

/* A hidden flag used by augparse to suppress loading of all the modules
   on the path */
#define AUG_NO_DEFAULT_LOAD (1 << 15)

/* The data structure representing a connection to Augeas. */
struct augeas {
    struct tree      *tree;
    const char       *root;       /* Filesystem root for all files */
                                  /* always ends with '/' */
    unsigned int      flags;      /* Flags passed to AUG_INIT */
    struct module    *modules;    /* Loaded modules */
    size_t            nmodpath;
    char             *modpathz;   /* The search path for modules as a
                                     glibc argz vector */
};

/* An entry in the global config tree. The data structure allows associating
 * values with interior nodes, but the API currently marks that as an error.
 */
struct tree {
    struct tree *next;
    const char  *label;      /* Last component of PATH */
    struct tree *children;   /* List of children through NEXT */
    const char  *value;
    int          dirty;
};

/* Allocate a new tree node with the given LABEL, VALUE, and CHILDREN,
 * which are not copied. The new tree is marked as dirty
 */
struct tree *make_tree(const char *label, const char *value,
                       struct tree *children);

int aug_tree_replace(struct augeas *aug, const char *path, struct tree *sub);

int tree_rm(struct tree **tree, const char *path);
struct tree *tree_set(struct tree *tree, const char *path, const char *value);
int free_tree(struct tree *tree);
void print_tree(struct tree *tree, FILE *out, const char *path, int pr_hidden);
int tree_equal(struct tree *t1, struct tree *t2);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

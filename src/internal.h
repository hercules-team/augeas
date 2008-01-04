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

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#ifdef __GNUC__
#ifdef HAVE_ANSIDECL_H
#include <ansidecl.h>
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* String equality tests, suggested by Jim Meyering. */
#define STREQ(a,b) (strcmp((a),(b)) == 0)
#define STRCASEEQ(a,b) (strcasecmp((a),(b)) == 0)
#define STRNEQ(a,b) (strcmp((a),(b)) != 0)
#define STRCASENEQ(a,b) (strcasecmp((a),(b)) != 0)
#define STREQLEN(a,b,n) (strncmp((a),(b),(n)) == 0)
#define STRNEQLEN(a,b,n) (strncmp((a),(b),(n)) != 0)

/* Call calloc to allocate an array of N instances of *VAR */
#define CALLOC(Var,N) do { (Var) = calloc ((N), sizeof (*(Var))); } while (0)

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
      fprintf(stderr, "%s:%d Unhandled error ",			\
              __FILE__, __LINE__);                      \
      fprintf(stderr, msg, ## args);                    \
      fputc('\n', stderr);                              \
    } while(0)

static inline void safe_free(void *p) {
    if(p)
        free(p);
}

/*
 * Internal data structures
 */

/* Size of the line buffer during parsing */
#define MAX_LINE 256

/*
 * File tokenizing
 */
enum aug_token_type {
    AUG_TOKEN_NONE,
    AUG_TOKEN_INERT,
    AUG_TOKEN_SEP,
    AUG_TOKEN_VALUE,
    AUG_TOKEN_EOR,
    AUG_TOKEN_EOF
};

struct aug_token {
    struct aug_token   *next;
    enum aug_token_type type;
    struct match       *match;
    const char         *text;
    const char         *node;  // The node associated with this token
};

struct aug_file {
    const char *name;  // The absolute file name
    const char *node;  // The node in the tree for this file
    struct aug_token *tokens;
    struct aug_file  *next;
};

// internal.c
void aug_token_free(struct aug_token *t);
void aug_file_free(struct aug_file *af);

/* Allocate a new file. NAME and NODE are dup'd */
struct aug_file *aug_make_file(const char *name, const char *node);

/* Allocate a new token. TEXT and NODE are not dup'd */
struct aug_token *aug_make_token(enum aug_token_type type,
                                 const char *text, const char *node);

struct aug_token *aug_insert_token(struct aug_token *t,
                                   enum aug_token_type type,
                                   const char *text,
                                   const char *node);

/* Append a new token to AF. The TEXT is added to the token without copying */
struct aug_token *aug_file_append_token(struct aug_file *af,
                                        enum aug_token_type type,
                                        const char *text,
                                        const char *node);

/* Read the contents of file PATH and return them as one long string. The
 * caller must free the result. Return NULL if any error occurs.
 */
const char* aug_read_file(const char *path);

// Defined in record.c
typedef struct aug_rec *aug_rec_t;

/*
 * A scanner describes how files are to be processed and keeps track of
 * where in the tree those files were put.  
 *
 * FIXME: Use of NODE is inconsistent (its a dir with files for the pam
 * provider), the direct file for the host provider
 */
struct aug_scanner {
    aug_rec_t  rec;
    const char *node;         // Node where the scanner is mounted
    struct aug_file  *files;
};

/*
 * Provider. Should eventually be the main interface between the tree
 * and the handling of individual config files
 */

struct aug_provider {
    const char *name;
    int (*init)(void);
    int (*load)(void);
    int (*save)(void);
};

/* Currently, all files have to be in this directory; this code is
 * definitely not suitable for dealing with live files
 */
#define ROOT_DIR "/tmp/aug"

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

/*
 * augrp.c: utility for printing and reading files as parsed by Augeas
 *
 * Copyright (C) 2007-2016 David Lutterkort
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
#include <argz.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>
#include <libgen.h>

#include "memory.h"
#include "augeas.h"
#include <locale.h>

#define EXIT_TROUBLE 2

#define cleanup(_x) __attribute__((__cleanup__(_x)))

const char *progname;
bool print_all = false;
bool print_only_values = false;
bool print_exact = false;

static void freep(void *p) {
    free(*(void **)p);
}

static void aug_closep(struct augeas **p) {
    aug_close(*p);
}

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] FILE\n", progname);
    fprintf(stderr,
"Print the contents of a file as parsed by augeas.\n\n"
"Options:\n\n"
"  -l, --lens LENS    use LENS to transform the file\n"
"  -L, --print-lens   print the lens that will be used for a file an exit\n"
"  -a, --all          print all nodes, even ones without a value\n"
"  -m, --match EXPR   start printing where nodes match EXPR\n"
"  -e, --exact        print only exact matches instead of the entire tree\n"
"                     starting at a match\n"
"  -o, --only-value   print only the values of tree nodes, but no path\n"
"  -q, --quiet        do not print anything. Exit with zero status if a\n"
"                     match was found\n"
"  -r, --root ROOT    use ROOT as the root of the filesystem\n"
"  -I, --include DIR  search DIR for modules; can be given mutiple times\n"
"  -S, --nostdinc     do not search the builtin default directories\n"
"                     for modules\n\n"
"Examples:\n\n"
"  Print how augeas sees /etc/exports:\n"
"    augmatch /etc/exports\n\n"
"  Show only the entry for a specific mount:\n"
"    augmatch -m 'dir[\"/home\"]' /etc/exports\n\n"
"  Show all the clients to which we are exporting /home:\n"
"    augmatch -eom 'dir[\"/home\"]/client' /etc/exports\n\n");
    exit(EXIT_SUCCESS);
}

/* Exit with a failure if COND is true. Use FORMAT and the remaining
 * arguments like printf to print an error message on stderr before
 * exiting */
static void die(bool cond, const char *format, ...) {
    if (cond) {
        fputs("error: ", stderr);
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
        exit(EXIT_TROUBLE);
    }
}

static void oom_when(bool cond) {
    die(cond, "out of memory.\n");
}

/* Format a string with vasprintf and return it. The caller is responsible
 * for freeing the result. */
static char *format(const char *format, ...) {
  va_list args;
  char *result;
  int r;

  va_start(args, format);
  r = vasprintf(&result, format, args);
  va_end(args);
  oom_when(r < 0);
  return result;
}

/* Check for an Augeas error. If there is one, print it and all its detail
 * and exit with failure. If there is no error, do nothing */
static void check_error(struct augeas *aug) {
    die(aug == NULL, "could not initialize augeas\n");
    oom_when(aug_error(aug) == AUG_ENOMEM);
    if (aug_error(aug) != AUG_NOERROR) {
        fprintf(stderr, "error: %s\n", aug_error_message(aug));
        const char *msg = aug_error_minor_message(aug);
        if (msg != NULL) {
            fprintf(stderr, "%s\n", msg);
        }
        msg = aug_error_details(aug);
        if (msg != NULL) {
            fprintf(stderr, "%s\n", msg);
        }
        exit(EXIT_TROUBLE);
    }
}

/* Check for an error trying to load FILE (e.g., a parse error) If there
 * was one, print details and exit with failure. If there was none, do
 * nothing. */
static void check_load_error(struct augeas *aug, const char *file) {
    char *info = format("/augeas/files%s", file);
    const char *msg, *line, *col;

    aug_defvar(aug, "info", info);
    free(info);
    die(aug_ns_count(aug, "info") == 0, "file %s does not exist\n", file);

    aug_defvar(aug, "error", "$info/error");
    if (aug_ns_count(aug, "error") == 0)
        return;

    aug_get(aug, "$error", &msg);
    aug_get(aug, "$error/line", &line);
    aug_get(aug, "$error/char", &col);

    if (streqv(msg, "parse_failed")) {
        msg = "parsing failed";
    } else if (streqv(msg, "read_failed")) {
        aug_get(aug, "$error/message", &msg);
    }

    if ((line != NULL) && (col != NULL)) {
        fprintf(stderr, "error reading %s: %s on line %s, column %s\n",
                file, msg, line, col);
    } else {
        fprintf(stderr, "error reading %s: %s\n", file, msg);
    }
    exit(EXIT_TROUBLE);
}

/* We keep track of where we are in the tree when we are printing it by
 * assigning to augeas variables and using one struct node for each level
 * in the tree. To keep things simple, we just preallocate a lot of them
 * (up to max_nodes many). If we ever have a tree deeper than this, we are
 * in trouble and will simply abort the program. */
static const size_t max_nodes = 256;

struct node {
    char *var;         /* The variable where we store the nodes for this level */
    const char *label; /* The label, index, and value of the current node */
    int   index;       /* at the level that this struct node is for */
    const char *value;
};

/* Print information about NODES[LEVEL] by printing the path to it going
 * from NODES[0] to NODES[LEVEL]. PREFIX is the path from the start of the
 * file to the current match (if the user specified --match) or the empty
 * string (if we are printing the entire file. */
static void print_one(int level, const char *prefix, struct node *nodes) {
    if (nodes[level].value == NULL && ! print_all)
        return;

    if (print_only_values && nodes[level].value != NULL) {
        printf("%s\n", nodes[level].value);
        return;
    }

    if (*prefix) {
        if (level > 0)
            printf("%s/", prefix);
        else
            printf("%s", prefix);
    }

    for (int i=1; i <= level; i++) {
        if (nodes[i].index > 0) {
            printf("%s[%d]", nodes[i].label, nodes[i].index);
        } else {
            printf("%s", nodes[i].label);
        }
        if (i < level) {
            printf("/");
        }
    }

    if (nodes[level].value) {
        printf(" = %s\n", nodes[level].value);
    } else {
        printf("\n");
    }
}

/* Recursively print the tree starting at NODES[LEVEL] */
static void print_tree(struct augeas *aug, int level,
                       const char *prefix, struct node *nodes) {
    die(level + 1 >= max_nodes,
        "tree has more than %d levels, which is more than we can handle\n",
        max_nodes);

    struct node *cur = nodes + level;
    struct node *next = cur + 1;

    int count = aug_ns_count(aug, cur->var);
    for (int i=0; i < count; i++) {
        cleanup(freep) char *pattern = NULL;

        aug_ns_label(aug, cur->var, i, &(cur->label), &(cur->index));
        aug_ns_value(aug, cur->var, i, &(cur->value));
        print_one(level, prefix, nodes);

        if (! print_exact) {
            pattern = format("$%s[%d]/*", cur->var, i+1);
            aug_defvar(aug, next->var, pattern);
            check_error(aug);
            print_tree(aug, level+1, prefix, nodes);
        }
    }
}

/* Print the tree for file PATH (which must already start with /files), but
 * only the nodes matching MATCH.
 *
 * Return EXIT_SUCCESS if there was at least one match, and EXIT_FAILURE
 * if there was none.
 */
static int print(struct augeas *aug, const char *path, const char *match) {
    static const char *const match_var = "match";

    struct node *nodes = NULL;

    nodes = calloc(max_nodes, sizeof(struct node));
    oom_when(nodes == NULL);

    for (int i=0; i < max_nodes; i++) {
        nodes[i].var = format("var%d", i);
    }

    /* Set $match to the nodes matching the user's match expression */
    aug_defvar(aug, match_var, match);
    check_error(aug);

    /* Go through the matches in MATCH_VAR one by one. We need to do it
     * this way, since the prefix we need to print for each entry in
     * MATCH_VAR is different for each entry. */
    int count = aug_ns_count(aug, match_var);
    for (int i=0; i < count; i++) {
        cleanup(freep) char *prefix = NULL;
        aug_ns_path(aug, match_var, i, &prefix);
        aug_defvar(aug, nodes[0].var, prefix);
        print_tree(aug, 0, prefix + strlen(path) + 1, nodes);
    }
    for (int i=0; i < max_nodes; i++) {
        free(nodes[i].var);
    }
    free(nodes);

    return (count == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}

/* Look at the filename and try to guess based on the extension. The
 * builtin filters for lenses do not do that, as that would force augtool
 * to scan everything on start
 */
static char *guess_lens_name(const char *file) {
    const char *ext = strrchr(file, '.');

    if (ext == NULL)
        return NULL;

    if (streqv(ext, ".json")) {
        return strdup("Json.lns");
    } else if (streqv(ext, ".xml")) {
        return strdup("Xml.lns");
    }

    return NULL;
}

int main(int argc, char **argv) {
    int opt;
    cleanup(aug_closep) struct augeas *aug;
    cleanup(freep) char *loadpath = NULL;
    size_t loadpath_len = 0;
    cleanup(freep) char *root = NULL;
    cleanup(freep) char *lens = NULL;
    cleanup(freep) char *matches = NULL;
    size_t matches_len = 0;
    const char *match = "*";
    bool print_lens = false;
    bool quiet = false;
    int result = EXIT_SUCCESS;

    struct option options[] = {
        { "help",       0, 0, 'h' },
        { "include",    1, 0, 'I' },
        { "lens",       1, 0, 'l' },
        { "all",        0, 0, 'a' },
        { "index",      0, 0, 'i' },
        { "match",      1, 0, 'm' },
        { "only-value", 0, 0, 'o' },
        { "nostdinc",   0, 0, 'S' },
        { "root",       1, 0, 'r' },
        { "print-lens", 0, 0, 'L' },
        { "exact",      0, 0, 'e' },
        { "quiet",      0, 0, 'q' },
        { 0, 0, 0, 0}
    };
    unsigned int flags = AUG_NO_LOAD|AUG_NO_ERR_CLOSE;
    progname = basename(argv[0]);

    setlocale(LC_ALL, "");
    while ((opt = getopt_long(argc, argv, "ahI:l:m:oSr:eLq", options, NULL)) != -1) {
        switch(opt) {
        case 'I':
            argz_add(&loadpath, &loadpath_len, optarg);
            break;
        case 'l':
            lens = strdup(optarg);
            break;
        case 'L':
            print_lens = true;
            break;
        case 'h':
            usage();
            break;
        case 'a':
            print_all = true;
            break;
        case 'm':
            // If optarg is a numeric string like '1', it is not a legal
            // part of a path by itself, and so we need to prefix it with
            // an explicit axis
            die(optarg[0] == '/',
                "matches can only be relative paths, not %s\n", optarg);
            argz_add(&matches, &matches_len, format("child::%s", optarg));
            break;
        case 'o':
            print_only_values = true;
            break;
        case 'r':
            root = strdup(optarg);
            break;
        case 'S':
            flags |= AUG_NO_STDINC;
            break;
        case 'e':
            print_exact = true;
            break;
        case 'q':
            quiet = true;
            break;
        default:
            fprintf(stderr, "Try '%s --help' for more information.\n",
                    progname);
            exit(EXIT_TROUBLE);
            break;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected an input file\n");
        fprintf(stderr, "Try '%s --help' for more information.\n",
                progname);
        exit(EXIT_TROUBLE);
    }

    const char *file = argv[optind];

    argz_stringify(loadpath, loadpath_len, ':');

    if (lens == NULL) {
        lens = guess_lens_name(file);
    }

    if (lens != NULL) {
        /* We know which lens we want, we do not need to load all of them */
        flags |= AUG_NO_MODL_AUTOLOAD;
    }

    aug = aug_init(root, loadpath, flags|AUG_NO_ERR_CLOSE);
    check_error(aug);

    if (lens == NULL) {
        aug_load_file(aug, file);
    } else {
        aug_transform(aug, lens, file, false);
        aug_load(aug);
    }
    check_error(aug);

    /* The user just wants the lens name */
    if (print_lens) {
        char *info = format("/augeas/files%s", file);
        const char *lens_name;
        aug_defvar(aug, "info", info);
        die(aug_ns_count(aug, "info") == 0,
            "file %s does not exist\n", file);
        aug_get(aug, "$info/lens", &lens_name);
        /* We are being extra careful here - the check_error above would
           have already aborted the program if we could not determine a
           lens; dieing here indicates some sort of bug */
        die(lens_name == NULL, "could not find lens for %s\n",
            file);
        if (lens_name[0] == '@')
            lens_name += 1;
        printf("%s\n", lens_name);
        exit(EXIT_SUCCESS);
    }

    check_load_error(aug, file);

    char *path = format("/files%s", file);
    aug_set(aug, "/augeas/context", path);


    if (matches_len > 0) {
        argz_stringify(matches, matches_len, '|');
        match = matches;
    }

    if (quiet) {
        int n = aug_match(aug, match, NULL);
        check_error(aug);
        result = (n == 0) ? EXIT_FAILURE : EXIT_SUCCESS;
    } else {
        result = print(aug, path, match);
    }
    free(path);

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

/*
 * dump.c:
 *
 * Copyright (C) 2009 Red Hat Inc.
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
 * Author: David Lutterkort <lutter@redhat.com>
 */

/*
 * Example program for dumping (part of) the Augeas tree.
 *
 * Run it as 'dump [-n] [pattern] > /tmp/out.txt' to dump all the nodes
 * matching PATTERN. The PATTERN '/files//descendant::*' dumps all nodes
 * from files, and '//descendant::*' dumps absolutely everything. If -n is
 * passed, uses a variable and aug_ns_* functions. Without -n, uses
 * aug_match and straight aug_get/aug_label/aug_source.
 *
 * You might have to set AUGEAS_ROOT and AUGEAS_LENS_LIB to point at the
 * right things. For example, to run dump against the tree that the Augeas
 * tests use, and to use the lenses in the checkout, run it as
 *    AUGEAS_ROOT=$PWD/tests/root AUGEAS_LENS_LIB=$PWD/lenses \
 *    dump '//descendant::*'
 *
 */

#include <augeas.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/*
 * Print out information for all nodes matching PATH using aug_match and
 * then aug_get etc. on each of the matches.
 */
static void dump_match(struct augeas *aug, const char *path) {
    char **matches;
    int nmatches;
    int i;

    nmatches = aug_match(aug, path, &matches);
    if (nmatches < 0) {
        fprintf(stderr, "aug_match for '%s' failed\n", path);
        fprintf(stderr, "error: %s\n", aug_error_message(aug));
        exit(1);
    }

    fprintf(stderr, "iterating matches\n");
    fprintf(stderr, "%d matches for %s\n", nmatches, path);

    for (i=0; i < nmatches; i++) {
        const char *value, *label;
        char *file;

        aug_get(aug, matches[i], &value);
        aug_label(aug, matches[i], &label);
        aug_source(aug, matches[i], &file);

        printf("%s: %s %s %s\n", matches[i], label, value, file);
        free(file);
        free(matches[i]);
    }
    free(matches);
}

/*
 * Print out information for all nodes matching PATH using aug_ns_*
 * functions
 */
static void dump_var(struct augeas *aug, const char *path) {
    int nmatches;
    int i;

    /* Define the variable 'matches' to hold all the nodes we are
       interested in */
    aug_defvar(aug, "matches", path);

    /* Count how many nodes we have */
    nmatches = aug_match(aug, "$matches", NULL);
    if (nmatches < 0) {
        fprintf(stderr, "aug_match for '%s' failed\n", path);
        fprintf(stderr, "error: %s\n", aug_error_message(aug));
        exit(1);
    }

    fprintf(stderr, "using var and aug_ns_*\n");
    fprintf(stderr, "%d matches for %s\n", nmatches, path);

    for (i=0; i < nmatches; i++) {
        const char *value, *label;
        char *file = NULL;

        /* Get information about the ith node, equivalent to calling
         * aug_get etc. for "$matches[i]" but much more efficient internally
         */
        aug_ns_attr(aug, "matches", i, &value, &label, &file);

        printf("%d: %s %s %s\n", i, label, value, file);
        free(file);
    }
}

static void print_time_taken(const struct timeval *start,
                             const struct timeval *stop) {
    time_t elapsed = (stop->tv_sec - start->tv_sec)*1000
                   + (stop->tv_usec - start->tv_usec)/1000;
    fprintf(stderr, "time: %ld ms\n", elapsed);
}

int main(int argc, char **argv) {
    int opt;
    int use_var = 0;
    const char *pattern = "/files//*";
    struct timeval stop, start;

    while ((opt = getopt(argc, argv, "n")) != -1) {
        switch (opt) {
        case 'n':
            use_var = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-n] [pattern]\n", argv[0]);
            fprintf(stderr, "      without '-n', iterate matches\n");
            fprintf(stderr, "      with '-n', use a variable and aug_ns_*\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    struct augeas *aug = aug_init(NULL, NULL, 0);

    if (optind < argc)
        pattern = argv[optind];

    gettimeofday(&start, NULL);
    if (use_var) {
        dump_var(aug, pattern);
    } else {
        dump_match(aug, pattern);
    }
    gettimeofday(&stop, NULL);
    print_time_taken(&start, &stop);
    return 0;
}

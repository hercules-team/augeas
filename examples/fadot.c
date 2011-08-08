/*
 * fadot.c: example usage of finite automata library
 *
 * Copyright (C) 2009, Francis Giraldeau
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
 * Author: Francis Giraldeau <francis.giraldeau@usherbrooke.ca>
 */

/*
 * The purpose of this example is to show the usage of libfa
 */

#include <config.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#include "fa.h"

const char *progname;

static const char *const escape_chars    = "\"\a\b\t\n\v\f\r\\";
static const char *const escape_names = "\"abtnvfr\\";

__attribute__((noreturn))
static void usage(void) {
  fprintf(stderr, "\nUsage: %s [OPTIONS] REGEXP\n", progname);
  fprintf(stderr, "\nCompile REGEXP and apply operation to them. By default, just print\n");
  fprintf(stderr, "the minimized regexp.\n");
  fprintf(stderr, "\nOptions:\n\n");
  fprintf(stderr, "  -o OPERATION       one of : show concat union intersect\n");
  fprintf(stderr, "                              complement minus example\n");
  fprintf(stderr, "  -f DOT_FILE        Path of output .dot file\n");
  fprintf(stderr, "  -n                 do not minimize resulting finite automaton\n");

  exit(EXIT_FAILURE);
}

int main (int argc, char **argv) {

  opterr = 0;

  int reduce = 1;
  char *file_output = NULL;
  const char *operation = NULL;
  FILE *fd;
  int c;
  int nb_regexp = 0;

  progname = argv[0];

  while ((c = getopt (argc, argv, "nhf:o:")) != -1)
    switch (c)
      {
      case 'n':
        reduce = 0;
        break;
      case 'f':
        file_output = optarg;
        break;
      case 'h':
        usage();
        break;
      case 'o':
        operation = optarg;
        break;
      case '?':
        if (optopt == 'o' || optopt == 'f')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        usage();
        break;
      default:
        usage();
        break;
      }

  //printf ("reduce = %d, file_output = %s, operation = %s\n",
  //        reduce, file_output, operation);

  if (operation == NULL)
    operation = "show";

  for (int i = optind; i < argc; i++) {
    nb_regexp++;
  }

  if (nb_regexp == 0) {
    printf("Please specify regexp to process.\n");
    usage();
  }

  struct fa* fa_result = NULL;

  if (!strcmp(operation,"show")) {
    fa_compile(argv[optind], strlen(argv[optind]), &fa_result);
  } else if (!strcmp(operation,"concat")) {

    if (nb_regexp < 2) {
      fprintf(stderr,"Please specify 2 or more regexp to concat");
      return 1;
    }

    fa_result = fa_make_basic(FA_EPSILON);
    struct fa* fa_tmp;
    for (int i = optind; i < argc; i++) {
      fa_compile(argv[i], strlen(argv[i]), &fa_tmp);
      fa_result = fa_concat(fa_result, fa_tmp);
    }

  } else if (!strcmp(operation, "union")) {

    if (nb_regexp < 2) {
      fprintf(stderr,"Please specify 2 or more regexp to union");
      return 1;
    }

    fa_result = fa_make_basic(FA_EMPTY);

    struct fa* fa_tmp;
    for (int i = optind; i < argc; i++) {
      fa_compile(argv[i], strlen(argv[i]), &fa_tmp);
      fa_result = fa_union(fa_result, fa_tmp);
    }

  } else if (!strcmp(operation, "intersect")) {

    if (nb_regexp < 2) {
      fprintf(stderr,"Please specify 2 or more regexp to intersect");
      return 1;
    }

    fa_compile(argv[optind], strlen(argv[optind]), &fa_result);
    struct fa* fa_tmp;

    for (int i = optind+1; i < argc; i++) {
      fa_compile(argv[i], strlen(argv[i]), &fa_tmp);
      fa_result = fa_intersect(fa_result, fa_tmp);
    }

  } else if (!strcmp(operation, "complement")) {

    if (nb_regexp >= 2) {
      fprintf(stderr,"Please specify one regexp to complement");
      return 1;
    }

    fa_compile(argv[optind], strlen(argv[optind]), &fa_result);
    fa_result = fa_complement(fa_result);

  } else if (!strcmp(operation, "minus")) {

    if (nb_regexp != 2) {
      fprintf(stderr,"Please specify 2 regexp for operation minus");
      return 1;
    }

    struct fa* fa_tmp1;
    struct fa* fa_tmp2;
    fa_compile(argv[optind], strlen(argv[optind]), &fa_tmp1);
    fa_compile(argv[optind+1], strlen(argv[optind+1]), &fa_tmp2);
    fa_result = fa_minus(fa_tmp1, fa_tmp2);

  } else if (!strcmp(operation, "example")) {

    if (nb_regexp != 1) {
      fprintf(stderr,"Please specify one regexp for operation example");
      return 1;
    }

    char* word = NULL;
    size_t word_len = 0;
    fa_compile(argv[optind], strlen(argv[optind]), &fa_result);
    fa_example(fa_result, &word, &word_len);
    printf("Example word = %s\n", word);

  }

  if (reduce) {
    fa_minimize(fa_result);
  }

  if (file_output != NULL) {
    if ((fd = fopen(file_output, "w")) == NULL) {
      fprintf(stderr, "Error while opening file %s \n", file_output);
      return 1;
    }

    fa_dot(fd, fa_result);
    fclose(fd);
  } else {
    int r;
    char *rx;
    size_t rx_len;

    r = fa_as_regexp(fa_result, &rx, &rx_len);
    if (r < 0) {
      fprintf(stderr, "Converting FA to regexp failed\n");
      return 1;
    }

    for (size_t i=0; i < rx_len; i++) {
      char *p;
      if (rx[i] && ((p = strchr(escape_chars, rx[i])) != NULL)) {
        printf("\\%c", escape_names[p - escape_chars]);
      } else if (! isprint(rx[i])) {
        printf("\\%030o", (unsigned char) rx[i]);
      } else {
        putchar(rx[i]);
      }
    }
    putchar('\n');
  }

  return 0;
}

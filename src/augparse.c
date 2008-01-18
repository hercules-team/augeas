/*
 * augparse.c: utility for parsing config files and seeing what's happening
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

#include "ast.h"
#include "list.h"

const char *progname;

static struct grammar *load_grammar(const char *name, FILE *log, int flags) {
    struct grammar *grammars = NULL;
    struct map     *maps = NULL;
    int r;
    
    r = load_spec(name, log, flags, &grammars, &maps);
    if (r == -1)
        exit(EXIT_FAILURE);
    
    /* FIXME: free maps (if any) */
    if (grammars == NULL) {
        fprintf(stderr, "No grammars found\n");
        exit(EXIT_FAILURE);
    }
    if (grammars->next != NULL) {
        fprintf(stderr, "Warning: multiple grammars found, only using %s\n",
                grammars->name);
    }
    return grammars;
}

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] GRAMMAR [FILE]\n", progname);
    fprintf(stderr, "Load GRAMMAR and parse FILE according to it.\n");
    fprintf(stderr, "If FILE is omitted, the GRAMMAR is read and printed\n");
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -P WHAT       Show details of how FILE is parsed. Possible values for WHAT\n"
                    "                are 'advance', 'match', 'tokens', 'actions', and 'ast'\n");
    fprintf(stderr, "  -G WHAT       Show details about GRAMMAR. Possible values for WHAT are\n"
                    "                'any', 'follow', 'first', 'actions', 'pretty' and 'all'\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int opt;
    int parse_flags = PF_NONE;
    int grammar_flags = GF_NONE;
    struct grammar *grammar;

    progname = argv[0];

    while ((opt = getopt(argc, argv, "hP:G:")) != -1) {
        switch(opt) {
        case 'P':
            if (STREQ(optarg, "advance"))
                parse_flags |= PF_ADVANCE;
            else if (STREQ(optarg, "match"))
                parse_flags |= PF_MATCH;
            else if (STREQ(optarg, "tokens"))
                parse_flags |= PF_TOKEN;
            else if (STREQ(optarg, "actions"))
                parse_flags |= PF_ACTION;
            else if (STREQ(optarg, "ast"))
                parse_flags |= PF_AST;
            else {
                fprintf(stderr, "Illegal argument '%s' for -%c\n", optarg, opt);
                usage();
            }
            break;
        case 'G':
            if (STREQ(optarg, "any"))
                grammar_flags |= GF_ANY_RE;
            else if (STREQ(optarg, "follow"))
                grammar_flags |= GF_FOLLOW;
            else if (STREQ(optarg, "first"))
                grammar_flags |= GF_FIRST;
            else if (STREQ(optarg, "actions"))
                grammar_flags |= GF_ACTIONS;
            else if (STREQ(optarg, "pretty"))
                grammar_flags |= GF_PRETTY;
            else if (STREQ(optarg, "all"))
                grammar_flags = ~ GF_NONE;
            else {
                fprintf(stderr, "Illegal argument '%s' for -%c\n", optarg, opt);
                usage();
            }
            break;
        case 'h':
            usage();
            break;
        default:
            usage();
            break;
        }
    }
    
    if (optind >= argc) {
        fprintf(stderr, "Expected grammar file\n");
        usage();
    }
    
    if (optind + 1 == argc) {
        if (grammar_flags == GF_NONE)
            grammar_flags |= GF_PRETTY;
        grammar = load_grammar(argv[optind], stdout, grammar_flags);
        if (grammar == NULL)
            return EXIT_FAILURE;
    } else {
        const char *text = aug_read_file(argv[optind+1]);
        struct aug_file *file = aug_make_file(argv[optind+1], "");

        if (text == NULL)
            return EXIT_FAILURE;

        grammar = load_grammar(argv[optind], stdout, grammar_flags);
        if (grammar == NULL)
            return EXIT_FAILURE;

        parse(grammar, file, text, stdout, parse_flags);
    }
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

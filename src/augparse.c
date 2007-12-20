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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "ast.h"

const char *progname;

static const char* load_file(const char *path) {
    FILE *fp = fopen(path, "r");
    struct stat st;
    char *result;

    if (!fp)
        return NULL;

    if (fstat(fileno(fp), &st) < 0) {
        fclose(fp);
        return NULL;
    }
    
    CALLOC(result, st.st_size + 1);
    if (result == NULL) {
        fclose(fp);
        return NULL;
    }

    if (st.st_size) {
        if (fread(result, st.st_size, 1, fp) != 1) {
            fclose(fp);
            free(result);
            return NULL;
        }
    }

    fclose(fp);
    return result;

}

static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] GRAMMAR [FILE]\n", progname);
    fprintf(stderr, "Load GRAMMAR and parses FILE according to it.\n");
    fprintf(stderr, "If FILE is omitted, the GRAMMAR is read and printed\n");
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -S WHAT       Show details of how FILE is parsed. Possible values for WHAT\n"
                    "                are 'advance', 'match', 'token', and 'rule'\n");
    fprintf(stderr, "  -G WHAT       Show details about GRAMMAR. Possible values for WHAT are\n"
                    "                'any', 'follow', 'first', 'nodes', 'pretty' and 'all'\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int opt;
    int parse_flags = PF_NONE;
    int grammar_flags = GF_NONE;
    struct grammar *grammar;

    progname = argv[0];

    while ((opt = getopt(argc, argv, "S:G:")) != -1) {
        switch(opt) {
        case 'S':
            if (STREQ(optarg, "advance"))
                parse_flags |= PF_ADVANCE;
            else if (STREQ(optarg, "match"))
                parse_flags |= PF_MATCH;
            else if (STREQ(optarg, "token"))
                parse_flags |= PF_TOKEN;
            else if (STREQ(optarg, "rule"))
                parse_flags |= PF_RULE;
            else {
                fprintf(stderr, "Illegal argument '%s' for -S\n", optarg);
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
            else if (STREQ(optarg, "nodes"))
                grammar_flags |= GF_NODES;
            else if (STREQ(optarg, "pretty"))
                grammar_flags |= GF_PRETTY;
            else if (STREQ(optarg, "all"))
                grammar_flags = ~ GF_NONE;
            else {
                fprintf(stderr, "Illegal argument '%s' for -S\n", optarg);
                usage();
            }
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
    } else {
        const char *text = load_file(argv[optind+1]);
        if (text == NULL)
            return 1;

        grammar = load_grammar(argv[optind], stdout, grammar_flags);
        if (grammar == NULL)
            return 1;

        parse(grammar, argv[optind+1], text, stdout, parse_flags);
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

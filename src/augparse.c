/*
 * augparse.c: utility for parsing config files and seeing what's happening
 *
 * Copyright (C) 2007, 2008 Red Hat Inc.
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

#include <argz.h>

#include "list.h"
#include "syntax.h"
#include "augeas.h"
#include "config.h"

const char *progname;

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] GRAMMAR [FILE]\n", progname);
    fprintf(stderr, "Load GRAMMAR and parse FILE according to it.\n");
    fprintf(stderr, "If FILE is omitted, the GRAMMAR is read and printed\n");
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -P WHAT       Show details of how FILE is parsed. Possible values for WHAT\n"
                    "                are 'advance', 'match', 'tokens', and 'skel'\n");
    fprintf(stderr, "  -I DIR        Add DIR to the module loadpath. Can be given multiple times.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int opt;
    int print_skels = 0;
    int parse_flags = PF_NONE;
    struct augeas *aug;
    char *loadpath = NULL;
    size_t loadpathlen = 0;

    progname = argv[0];

    while ((opt = getopt(argc, argv, "hP:I:")) != -1) {
        switch(opt) {
        case 'P':
            if (STREQ(optarg, "advance"))
                parse_flags |= PF_ADVANCE;
            else if (STREQ(optarg, "match"))
                parse_flags |= PF_MATCH;
            else if (STREQ(optarg, "tokens"))
                parse_flags |= PF_TOKEN;
            else if (STREQ(optarg, "skel"))
                print_skels = 1;
            else {
                fprintf(stderr, "Illegal argument '%s' for -%c\n", optarg, opt);
                usage();
            }
            break;
        case 'I':
            argz_add(&loadpath, &loadpathlen, optarg);
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
        fprintf(stderr, "Expected .aug file\n");
        usage();
    }

    argz_stringify(loadpath, loadpathlen, PATH_SEP_CHAR);
    aug = aug_init(NULL, loadpath, AUG_TYPE_CHECK|AUG_NO_DEFAULT_LOAD);
    if (__aug_load_module_file(aug, argv[optind]) == -1) {
        fprintf(stderr, "%s: error: Loading failed\n", argv[optind]);
        exit(EXIT_FAILURE);
    }
    aug_close(aug);
    //if (optind + 1 < argc) {
        //const char *text = aug_read_file(argv[optind+1]);
        // Run text through some sort of lens
    //}
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

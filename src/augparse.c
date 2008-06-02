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

const char *progname;

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] MODULE\n", progname);
    fprintf(stderr, "Evaluate MODULE. Generally, MODULE should contain unit tests.\n");
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -I DIR        Add DIR to the module loadpath. Can be given multiple times.\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int opt;
    struct augeas *aug;
    char *loadpath = NULL;
    size_t loadpathlen = 0;

    progname = argv[0];

    while ((opt = getopt(argc, argv, "hI:")) != -1) {
        switch(opt) {
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
    free(loadpath);
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

/*
 * augparse.c: utility for parsing config files and seeing what's happening
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

#include <config.h>
#include <argz.h>
#include <getopt.h>

#include "list.h"
#include "syntax.h"
#include "augeas.h"
#include <locale.h>

const char *progname;
bool print_version = false;

__attribute__((noreturn))
static void usage(void) {
    fprintf(stderr, "Usage: %s [OPTIONS] MODULE\n", progname);
    fprintf(stderr, "Evaluate MODULE. Generally, MODULE should contain unit tests.\n");
    fprintf(stderr, "\nOptions:\n\n");
    fprintf(stderr, "  -I, --include DIR  search DIR for modules; can be given mutiple times\n");
    fprintf(stderr, "  --nostdinc         do not search the builtin default directories for modules\n");
    fprintf(stderr, "  --notypecheck      do not typecheck lenses\n");
    fprintf(stderr, "  --version          print version information and exit\n");

    exit(EXIT_FAILURE);
}

static void print_version_info(struct augeas *aug) {
    const char *version;
    int r;

    r = aug_get(aug, "/augeas/version", &version);
    if (r != 1)
        goto error;

    fprintf(stderr, "augparse %s <http://augeas.net/>\n", version);
    fprintf(stderr, "Copyright (C) 2007-2011 David Lutterkort\n");
    fprintf(stderr, "License LGPLv2+: GNU LGPL version 2.1 or later\n");
    fprintf(stderr, "                 <http://www.gnu.org/licenses/lgpl-2.1.html>\n");
    fprintf(stderr, "This is free software: you are free to change and redistribute it.\n");
    fprintf(stderr, "There is NO WARRANTY, to the extent permitted by law.\n\n");
    fprintf(stderr, "Written by David Lutterkort\n");
    return;
 error:
    fprintf(stderr, "Something went terribly wrong internally - please file a bug\n");
}

int main(int argc, char **argv) {
    int opt;
    struct augeas *aug;
    char *loadpath = NULL;
    size_t loadpathlen = 0;
    enum {
        VAL_NO_STDINC = CHAR_MAX + 1,
        VAL_NO_TYPECHECK = VAL_NO_STDINC + 1,
        VAL_VERSION = VAL_NO_TYPECHECK + 1
    };
    struct option options[] = {
        { "help",      0, 0, 'h' },
        { "include",   1, 0, 'I' },
        { "nostdinc",  0, 0, VAL_NO_STDINC },
        { "notypecheck",  0, 0, VAL_NO_TYPECHECK },
        { "version",  0, 0, VAL_VERSION },
        { 0, 0, 0, 0}
    };
    int idx;
    unsigned int flags = AUG_TYPE_CHECK|AUG_NO_MODL_AUTOLOAD;
    progname = argv[0];

    setlocale(LC_ALL, "");
    while ((opt = getopt_long(argc, argv, "hI:", options, &idx)) != -1) {
        switch(opt) {
        case 'I':
            argz_add(&loadpath, &loadpathlen, optarg);
            break;
        case 'h':
            usage();
            break;
        case VAL_NO_STDINC:
            flags |= AUG_NO_STDINC;
            break;
        case VAL_NO_TYPECHECK:
            flags &= ~(AUG_TYPE_CHECK);
            break;
        case VAL_VERSION:
            print_version = true;
            break;
        default:
            usage();
            break;
        }
    }

    if (!print_version && optind >= argc) {
        fprintf(stderr, "Expected .aug file\n");
        usage();
    }

    argz_stringify(loadpath, loadpathlen, PATH_SEP_CHAR);
    aug = aug_init(NULL, loadpath, flags);
    if (aug == NULL) {
        fprintf(stderr, "Memory exhausted\n");
        return 2;
    }

    if (print_version) {
        print_version_info(aug);
        return EXIT_SUCCESS;
    }

    if (__aug_load_module_file(aug, argv[optind]) == -1) {
        fprintf(stderr, "%s\n", aug_error_message(aug));
        const char *s = aug_error_details(aug);
        if (s != NULL) {
            fprintf(stderr, "%s\n", s);
        }
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

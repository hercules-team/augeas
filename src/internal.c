/*
 * internal.c: internal data structures and helpers
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

#include "internal.h"

void aug_file_free(struct aug_file *af) {
    if (af != NULL) {
        safe_free((void *) af->name);
        safe_free((void *) af->node);
        /* FIXME: free af->ast */
    }
}

struct aug_file *aug_make_file(const char *name, const char *node) {
    struct aug_file *result;

    result = calloc(1, sizeof(struct aug_file));
    if (result == NULL)
        return NULL;

    result->name = strdup(name);
    result->node = strdup(node);
    if (result->name == NULL || result->node == NULL) {
        free(result);
        return NULL;
    }

    return result;
}

const char* aug_read_file(const char *path) {
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
    result[st.st_size] = '\0';

    fclose(fp);
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

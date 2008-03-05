/*
 * augeas.h: public headers for augeas
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

#include <stdio.h>

#ifndef __AUGEAS_H
#define __AUGEAS_H

typedef struct augeas *augeas_t;

/* Initialize the library; returns -1 on error, 0 on success */
augeas_t aug_init(void);

/* Lookup the value associated with PATH */
const char *aug_get(augeas_t aug, const char *path);

/* Set the value associated with PATH to VALUE. VALUE is copied into the
   internal data structure. Intermediate entries are created if they don't
   exist. Return -1 on error, 0 on success */
int aug_set(augeas_t aug, const char *path, const char *value);

/* Return 1 if there is an entry for this path, 0 otherwise */
int aug_exists(augeas_t aug, const char *path);

/* Make PATH a SIBLING of PATH by inserting it directly before SIBLING. */
int aug_insert(augeas_t aug, const char *path, const char *sibling);

/* Remove path and all its children. Returns the number of entries removed */
int aug_rm(augeas_t aug, const char *path);

/* Return a list of the direct children of PATH in CHILDREN, which is
   allocated and must be freed by the caller, including the strings it
   contains. If CHILDREN is NULL, nothing is allocated and only the number
   of children is returned. Returns -1 on error, or the total number of
   children of PATH.
*/
int aug_ls(augeas_t aug, const char *path, const char ***children);

/* Return the first SIZE paths that match PATTERN in MATCHES, which must be
 * preallocated to hold at least SIZE entries. The return value is the total
 * number of matches. Any strings returned in MATCHES must be freed by the
 * caller.
 *
 * The PATTERN is passed to fnmatch(3) verbatim, and FNM_FILE_NAME is not set,
 * so that '*' does not match a '/'
 */
int aug_match(augeas_t aug, const char *pattern,
              const char **matches, int size);

/* Write all pending changes to disk. Return -1 if an error is encountered,
 * 0 on success.
 */
int aug_save(augeas_t aug);

/* Print the subtree starting at PATH to OUT */
void aug_print(augeas_t aug, FILE *out, const char *path);

/* Close this Augeas instance and free any storage associated with
 * it. After running AUG_CLOSE, AUG is invalid and can not be used for any
 * more operations.
 */
void aug_close(augeas_t aug);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

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

/* Initialize the library; returns -1 on error, 0 on success */
int aug_init(void);

/* Lookup the value associated with PATH */
const char *aug_lookup(const char *path);

/* Set the value associated with PATH to VALUE. VALUE is copied into the
   internal data structure. Intermediate entries are created if they don't
   exist. Return -1 on error, 0 on success */
int aug_set(const char *path, const char *value);

/* Make PATH a SIBLING of PATH by inserting it directly before SIBLING. */
int aug_insert(const char *path, const char *sibling);

/* Remove path and all its children. Returns the number of entries removed */
int aug_rm(const char *path);

/* Return a list of the direct children of PATH in CHILDREN, which must 
   be big enough to hold SIZE entries. Returns -1 on error, or the total 
   number of children of PATH. If SIZE is smaller than the total number of
   children, only the first SIZE are put into CHILDREN
*/
int aug_ls(const char *path, const char **children, int size);

/* Write all pending changes to disk */
int aug_commit(void);

void aug_dump(FILE *out);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

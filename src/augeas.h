/*
 * augeas.h: public headers for augeas
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

#include <stdio.h>

#ifndef AUGEAS_H_
#define AUGEAS_H_

typedef struct augeas augeas;

/* Flags to influence the behavior of Augeas. Pass a bitmask of these flags
 * to AUG_INIT.
 */
enum aug_flags {
    AUG_NONE = 0,
    AUG_SAVE_BACKUP  = (1 << 0),  /* Keep the original file with a
                                     .augsave extension */
    AUG_SAVE_NEWFILE = (1 << 1),  /* Save changes into a file with
                                     extension .augnew, and do not
                                     overwrite the original file. Takes
                                     precedence over AUG_SAVE_BACKUP */
    AUG_TYPE_CHECK   = (1 << 2)   /* Typecheck lenses; since it can be very
                                     expensive it is not done by default */
};

/* Initialize the library.
 *
 * Use ROOT as the filesystem root. If ROOT is NULL, use the value of the
 * environment variable AUGEAS_ROOT. If that doesn't exist eitehr, use "/".
 *
 * LOADPATH is a colon-spearated list of directories that modules should be
 * searched in. This is in addition to the standard load path and the
 * directories in AUGEAS_LENS_LIB
 *
 * FLAGS is a bitmask made up of values from AUG_FLAGS.
 *
 * Return a handle to the Augeas tree upon success. If initialization
 * fails, returns NULL.
 */
augeas *aug_init(const char *root, const char *loadpath, unsigned int flags);

/* Lookup the value associated with PATH. Return NULL if PATH does not
 * exist, or if it matches more than one node, or if that is the value
 * associated with the single node matched by PATH.
 *
 * See AUG_EXISTS on how to tell these cases apart.
 */
const char *aug_get(const augeas *aug, const char *path);

/* Set the value associated with PATH to VALUE. VALUE is copied into the
 * internal data structure. Intermediate entries are created if they don't
 * exist. Return 0 on success, -1 on error. It is an error if more than one
 * node matches PATH.
 */
int aug_set(augeas *aug, const char *path, const char *value);

/* Return 1 if there is exactly one node matching PATH, 0 if there is none,
 * and -1 if there is more than one node matching PATH. You should only
 * call AUG_GET for paths for which AUG_EXISTS returns 1, and AUG_SET for
 * paths for which AUG_EXISTS returns 0 or 1.
 */
int aug_exists(const augeas *aug, const char *path);

/* Create a new sibling LABEL for PATH by inserting into the tree just
 * before PATH if BEFORE == 1 or just after PATH if BEFORE == 0.
 *
 * PATH must match exactly one existing node in the tree, and LABEL must be
 * a label, i.e. not contain a '/', '*' or end with a bracketed index
 * '[N]'.
 *
 * Return 0 on success, and -1 if the insertion fails.
 */
int aug_insert(augeas *aug, const char *path, const char *label, int before);

/* Remove path and all its children. Returns the number of entries removed.
 * All nodes that match PATH, and their descendants, are removed.
 */
int aug_rm(augeas *aug, const char *path);

/* Return the number of matches of the path expression PATH in AUG. If
 * MATCHES is non-NULL, an array with the returned number of elements will
 * be allocated and filled with the paths of the matches. The caller must
 * free both the array and the entries in it. The returned paths are
 * sufficiently qualified to make sure that they match exactly one node in
 * the current tree.
 *
 * If MATCHES is NULL, nothing is allocated and only the number
 * of matches is returned.
 *
 * Returns -1 on error, or the total number of matches (which might be 0).
 *
 * Path expressions use a very simple subset of XPath: the path PATH
 * consists of a number of segments, separated by '/'; each segment can
 * either be a '*', matching any tree node, or a string, optionally
 * followed by an index in brackets, matching tree nodes labelled with
 * exactly that string. If no index is specified, the expression matches
 * all nodes with that label; the index can be a positive number N, which
 * matches exactly the Nth node with that label (counting from 1), or the
 * special expression 'last()' which matches the last node with the given
 * label. All matches are done in fixed positions in the tree, and nothing
 * matches more than one path segment.
 *
 */
int aug_match(const augeas *aug, const char *path, char ***matches);

/* Write all pending changes to disk. Return -1 if an error is encountered,
 * 0 on success. Only files that had any changes made to them are written.
 *
 * If AUG_SAVE_NEWFILE is set in the FLAGS passed to AUG_INIT, create
 * changed files as new files with the extension ".augnew", and leave teh
 * original file unmodified.
 *
 * Otherwise, if AUG_SAVE_BACKUP is set in the FLAGS passed to AUG_INIT,
 * move the original file to a new file with extension ".augsave".
 *
 * If neither of these flags is set, overwrite the original file.
 */
int aug_save(augeas *aug);

/* Print each node matching PATH and its descendants to OUT.
 * Return 0 on success, or a negative value on failure
 */
int aug_print(const augeas *aug, FILE *out, const char *path);

/* Close this Augeas instance and free any storage associated with
 * it. After running AUG_CLOSE, AUG is invalid and can not be used for any
 * more operations.
 */
void aug_close(augeas *aug);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

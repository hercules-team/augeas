/*
 * Copyright (C) 2009-2011 David Lutterkort
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
 * Author: David Lutterkort <lutter@redhat.com>
 */

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

/*
 * Transformers for going from file globs to path names in the tree
 * functions are in transform.c
 */

/* Filters for globbing files */
struct filter {
    unsigned int   ref;
    struct filter *next;
    struct string *glob;
    unsigned int   include : 1;
};

struct filter *make_filter(struct string *glb, unsigned int include);
void free_filter(struct filter *filter);

/* Transformers that actually run lenses on contents of files */
struct transform {
    unsigned int      ref;
    struct lens      *lens;
    struct filter    *filter;
};

struct transform *make_transform(struct lens *lens, struct filter *filter);
void free_transform(struct transform *xform);

/*
 * When we pass a tree for a transform, the tree must have exactly one
 * child with label "lens" whose value is the qualified name of the lens to
 * use, and any number of children labelled "incl" or "excl" whose values
 * are glob patterns used to filter which files to transform.
 */

/* Verify that the tree XFM represents a valid transform. If it does not,
 * add an 'error' child to it.
 *
 * Return 0 if XFM is a valid transform, -1 otherwise.
 */
int transform_validate(struct augeas *aug, struct tree *xfm);

/* Load all files matching the TRANSFORM's filter into the tree in AUG by
 * applying the TRANSFORM's lens to their contents and putting the
 * resulting tree under "/files" + filename. Also stores some information
 * about filename underneath "/augeas/files" + filename
 */
int transform_load(struct augeas *aug, struct tree *xfm);

/* Return 1 if TRANSFORM applies to PATH, 0 otherwise. The TRANSFORM
 * applies to PATH if (1) PATH starts with "/files/" and (2) the rest of
 * PATH matches the transform's filter
*/
int transform_applies(struct tree *xfm, const char *path);

/* Save TREE into the file corresponding to PATH. It is assumed that the
 * TRANSFORM applies to that PATH
 */
int transform_save(struct augeas *aug, struct tree *xfm,
                   const char *path, struct tree *tree);

/* Remove the file for TREE, either by moving it to a .augsave file or by
 * unlinking it, depending on aug->flags. TREE must be the node underneath
 * /augeas/files corresponding to the file to be removed.
 *
 * Return 0 on success, -1 on failure
 */
int remove_file(struct augeas *aug, struct tree *tree);

/* Return a printable name for the transform XFM. Never returns NULL. */
const char *xfm_lens_name(struct tree *xfm);

/* Store a file-specific transformation error in /augeas/files/PATH/error */
ATTRIBUTE_FORMAT(printf, 4, 5)
void transform_file_error(struct augeas *aug, const char *status,
                          const char *filename, const char *format, ...);
#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

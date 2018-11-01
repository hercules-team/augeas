/*
 * augeas.h: public headers for augeas
 *
 * Copyright (C) 2007-2016 David Lutterkort
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
#include <libxml/tree.h>

#ifndef AUGEAS_H_
#define AUGEAS_H_

typedef struct augeas augeas;

/* Enum: aug_flags
 *
 * Flags to influence the behavior of Augeas. Pass a bitmask of these flags
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
    AUG_TYPE_CHECK   = (1 << 2),  /* Typecheck lenses; since it can be very
                                     expensive it is not done by default */
    AUG_NO_STDINC    = (1 << 3),   /* Do not use the builtin load path for
                                     modules */
    AUG_SAVE_NOOP    = (1 << 4),  /* Make save a no-op process, just record
                                     what would have changed */
    AUG_NO_LOAD      = (1 << 5),  /* Do not load the tree from AUG_INIT */
    AUG_NO_MODL_AUTOLOAD = (1 << 6),
    AUG_ENABLE_SPAN  = (1 << 7),  /* Track the span in the input of nodes */
    AUG_NO_ERR_CLOSE = (1 << 8),  /* Do not close automatically when
                                     encountering error during aug_init */
    AUG_TRACE_MODULE_LOADING = (1 << 9) /* For use by augparse -t */
};

#ifdef __cplusplus
extern "C" {
#endif

/* Function: aug_init
 *
 * Initialize the library.
 *
 * Use ROOT as the filesystem root. If ROOT is NULL, use the value of the
 * environment variable AUGEAS_ROOT. If that doesn't exist eitehr, use "/".
 *
 * LOADPATH is a colon-separated list of directories that modules should be
 * searched in. This is in addition to the standard load path and the
 * directories in AUGEAS_LENS_LIB. LOADPATH can be NULL, indicating that
 * nothing should be added to the load path.
 *
 * FLAGS is a bitmask made up of values from AUG_FLAGS. The flag
 * AUG_NO_ERR_CLOSE can be used to get more information on why
 * initialization failed. If it is set in FLAGS, the caller must check that
 * aug_error returns AUG_NOERROR before using the returned augeas handle
 * for any other operation. If the handle reports any error, the caller
 * should only call the aug_error functions an aug_close on this handle.
 *
 * Returns:
 * a handle to the Augeas tree upon success. If initialization fails,
 * returns NULL if AUG_NO_ERR_CLOSE is not set in FLAGS. If
 * AUG_NO_ERR_CLOSE is set, might return an Augeas handle even on
 * failure. In that case, caller must check for errors using augeas_error,
 * and, if an error is reported, only use the handle with the aug_error
 * functions and aug_close.
 */
augeas *aug_init(const char *root, const char *loadpath, unsigned int flags);

/* Function: aug_defvar
 *
 * Define a variable NAME whose value is the result of evaluating EXPR. If
 * a variable NAME already exists, its name will be replaced with the
 * result of evaluating EXPR.  Context will not be applied to EXPR.
 *
 * If EXPR is NULL, the variable NAME will be removed if it is defined.
 *
 * Path variables can be used in path expressions later on by prefixing
 * them with '$'.
 *
 * Returns -1 on error; on success, returns 0 if EXPR evaluates to anything
 * other than a nodeset, and the number of nodes if EXPR evaluates to a
 * nodeset
 */
int aug_defvar(augeas *aug, const char *name, const char *expr);

/* Function: aug_defnode
 *
 * Define a variable NAME whose value is the result of evaluating EXPR,
 * which must be non-NULL and evaluate to a nodeset. If a variable NAME
 * already exists, its name will be replaced with the result of evaluating
 * EXPR.
 *
 * If EXPR evaluates to an empty nodeset, a node is created, equivalent to
 * calling AUG_SET(AUG, EXPR, VALUE) and NAME will be the nodeset containing
 * that single node.
 *
 * If CREATED is non-NULL, it is set to 1 if a node was created, and 0 if
 * it already existed.
 *
 * Returns -1 on error; on success, returns the number of nodes in the
 * nodeset
 */
int aug_defnode(augeas *aug, const char *name, const char *expr,
                const char *value, int *created);

/* Function: aug_get
 *
 * Lookup the value associated with PATH. VALUE can be NULL, in which case
 * it is ignored. If VALUE is not NULL, it is used to return a pointer to
 * the value associated with PATH if PATH matches exactly one node. If PATH
 * matches no nodes or more than one node, *VALUE is set to NULL. Note that
 * it is perfectly legal for nodes to have a NULL value, and that that by
 * itself does not indicate an error.
 *
 * The string *VALUE must not be freed by the caller, and is valid as long
 * as its node remains unchanged.
 *
 * Returns:
 * 1 if there is exactly one node matching PATH, 0 if there is none,
 * and a negative value if there is more than one node matching PATH, or if
 * PATH is not a legal path expression.
 */
int aug_get(const augeas *aug, const char *path, const char **value);

/* Function: aug_label
 *
 * Lookup the label associated with PATH. LABEL can be NULL, in which case
 * it is ignored. If LABEL is not NULL, it is used to return a pointer to
 * the value associated with PATH if PATH matches exactly one node. If PATH
 * matches no nodes or more than one node, *LABEL is set to NULL.
 *
 * The string *LABEL must not be freed by the caller, and is valid as long
 * as its node remains unchanged.
 *
 * Returns:
 * 1 if there is exactly one node matching PATH, 0 if there is none,
 * and a negative value if there is more than one node matching PATH, or if
 * PATH is not a legal path expression.
 */
int aug_label(const augeas *aug, const char *path, const char **label);

/* Function: aug_set
 *
 * Set the value associated with PATH to VALUE. VALUE is copied into the
 * internal data structure, and the caller is responsible for freeing
 * it. Intermediate entries are created if they don't exist.
 *
 * Returns:
 * 0 on success, -1 on error. It is an error if more than one node
 * matches PATH.
 */
int aug_set(augeas *aug, const char *path, const char *value);

/* Function: aug_setm
 *
 * Set the value of multiple nodes in one operation. Find or create a node
 * matching SUB by interpreting SUB as a path expression relative to each
 * node matching BASE. SUB may be NULL, in which case all the nodes
 * matching BASE will be modified.
 *
 * Returns:
 * number of modified nodes on success, -1 on error
 */
int aug_setm(augeas *aug, const char *base, const char *sub, const char *value);

/* Function: aug_span
 *
 * Get the span according to input file of the node associated with PATH. If
 * the node is associated with a file, the filename, label and value start and
 * end positions are set, and return value is 0. The caller is responsible for
 * freeing returned filename. If an argument for return value is NULL, then the
 * corresponding value is not set. If the node associated with PATH doesn't
 * belong to a file or is doesn't exists, filename and span are not set and
 * return value is -1.
 *
 * Returns:
 * 0 on success with filename, label_start, label_stop, value_start, value_end,
 *   span_start, span_end
 * -1 on error
 */

int aug_span(augeas *aug, const char *path, char **filename,
        unsigned int *label_start, unsigned int *label_end,
        unsigned int *value_start, unsigned int *value_end,
        unsigned int *span_start, unsigned int *span_end);

/* Function: aug_insert
 *
 * Create a new sibling LABEL for PATH by inserting into the tree just
 * before PATH if BEFORE == 1 or just after PATH if BEFORE == 0.
 *
 * PATH must match exactly one existing node in the tree, and LABEL must be
 * a label, i.e. not contain a '/', '*' or end with a bracketed index
 * '[N]'.
 *
 * Returns:
 * 0 on success, and -1 if the insertion fails.
 */
int aug_insert(augeas *aug, const char *path, const char *label, int before);

/* Function: aug_rm
 *
 * Remove path and all its children. Returns the number of entries removed.
 * All nodes that match PATH, and their descendants, are removed.
 */
int aug_rm(augeas *aug, const char *path);

/* Function: aug_mv
 *
 * Move the node SRC to DST. SRC must match exactly one node in the
 * tree. DST must either match exactly one node in the tree, or may not
 * exist yet. If DST exists already, it and all its descendants are
 * deleted. If DST does not exist yet, it and all its missing ancestors are
 * created.
 *
 * Note that the node SRC always becomes the node DST: when you move /a/b
 * to /x, the node /a/b is now called /x, no matter whether /x existed
 * initially or not.
 *
 * Returns:
 * 0 on success and -1 on failure.
 */
int aug_mv(augeas *aug, const char *src, const char *dst);

/* Function: aug_cp
 *
 * Copy the node SRC to DST. SRC must match exactly one node in the
 * tree. DST must either match exactly one node in the tree, or may not
 * exist yet. If DST exists already, it and all its descendants are
 * deleted. If DST does not exist yet, it and all its missing ancestors are
 * created.
 *
 * Returns:
 * 0 on success and -1 on failure.
 */
int aug_cp(augeas *aug, const char *src, const char *dst);

/* Function: aug_rename
 *
 * Rename the label of all nodes matching SRC to LBL.
 *
 * Returns:
 * The number of nodes renamed on success and -1 on failure.
 */
int aug_rename(augeas *aug, const char *src, const char *lbl);

/* Function: aug_match
 *
 * Returns:
 * the number of matches of the path expression PATH in AUG. If
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
 * Path expressions:
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

/* Function: aug_save
 *
 * Write all pending changes to disk.
 *
 * Returns:
 * -1 if an error is encountered,
 * 0 on success. Only files that had any changes made to them are written.
 *
 * If AUG_SAVE_NEWFILE is set in the FLAGS passed to AUG_INIT, create
 * changed files as new files with the extension ".augnew", and leave the
 * original file unmodified.
 *
 * Otherwise, if AUG_SAVE_BACKUP is set in the FLAGS passed to AUG_INIT,
 * move the original file to a new file with extension ".augsave".
 *
 * If neither of these flags is set, overwrite the original file.
 */
int aug_save(augeas *aug);

/* Function: aug_load
 *
 * Load files into the tree. Which files to load and what lenses to use on
 * them is specified under /augeas/load in the tree; each entry
 * /augeas/load/NAME specifies a 'transform', by having itself exactly one
 * child 'lens' and any number of children labelled 'incl' and 'excl'. The
 * value of NAME has no meaning.
 *
 * The 'lens' grandchild of /augeas/load specifies which lens to use, and
 * can either be the fully qualified name of a lens 'Module.lens' or
 * '@Module'. The latter form means that the lens from the transform marked
 * for autoloading in MODULE should be used.
 *
 * The 'incl' and 'excl' grandchildren of /augeas/load indicate which files
 * to transform. Their value are used as glob patterns. Any file that
 * matches at least one 'incl' pattern and no 'excl' pattern is
 * transformed. The order of 'incl' and 'excl' entries is irrelevant.
 *
 * When AUG_INIT is first called, it populates /augeas/load with the
 * transforms marked for autoloading in all the modules it finds.
 *
 * Before loading any files, AUG_LOAD will remove everything underneath
 * /augeas/files and /files, regardless of whether any entries have been
 * modified or not.
 *
 * Returns -1 on error, 0 on success. Note that success includes the case
 * where some files could not be loaded. Details of such files can be found
 * as '/augeas//error'.
 */
int aug_load(augeas *aug);

/* Function: aug_text_store
 *
 * Use the value of node NODE as a string and transform it into a tree
 * using the lens LENS and store it in the tree at PATH, which will be
 * overwritten. PATH and NODE are path expressions.
 *
 * Returns:
 * 0 on success, or a negative value on failure
 */
int aug_text_store(augeas *aug, const char *lens, const char *node,
                   const char *path);

/* Function: aug_text_retrieve
 *
 * Transform the tree at PATH into a string using lens LENS and store it in
 * the node NODE_OUT, assuming the tree was initially generated using the
 * value of node NODE_IN. PATH, NODE_IN, and NODE_OUT are path expressions.
 *
 * Returns:
 * 0 on success, or a negative value on failure
 */
int aug_text_retrieve(struct augeas *aug, const char *lens,
                      const char *node_in, const char *path,
                      const char *node_out);

/* Function: aug_escape_name
 *
 * Escape special characters in a string such that it can be used as part
 * of a path expressions and only matches a node named exactly
 * IN. Characters that have special meanings in path expressions, such as
 * '[' and ']' are prefixed with a '\\'. Note that this function assumes
 * that it is passed a name, not a path, and will therefore escape '/',
 * too.
 *
 * On return, *OUT is NULL if IN does not need any escaping at all, and
 * points to an escaped copy of IN otherwise.
 *
 * Returns:
 * 0 on success, or a negative value on failure
 */
int aug_escape_name(augeas *aug, const char *in, char **out);

/* Function: aug_print
 *
 * Print each node matching PATH and its descendants to OUT.
 *
 * Returns:
 * 0 on success, or a negative value on failure
 */
int aug_print(const augeas *aug, FILE *out, const char *path);

/* Function: aug_source
 *
 * For the node matching PATH, return the path to the node representing the
 * file to which PATH belongs. If PATH belongs to a file, *FILE_PATH will
 * contain the path to the toplevel node of that file underneath /files. If
 * it does not, *FILE_PATH will be NULL.
 *
 * The caller is responsible for freeing *FILE_PATH
 *
 * Returns:
 * 0 on success, or a negative value on failure. It is an error if PATH
 * matches more than one node.
 */
int aug_source(const augeas *aug, const char *path, char **file_path);

/* Function: aug_to_xml
 *
 * Turn the Augeas tree(s) matching PATH into an XML tree XMLDOC. The
 * parameter FLAGS is currently unused and must be set to 0.
 *
 * Returns:
 * 0 on success, or a negative value on failure
 *
 * In case of failure, *xmldoc is set to NULL
 */
int aug_to_xml(const augeas *aug, const char *path, xmlNode **xmldoc,
               unsigned int flags);

/*
 * Function: aug_transform
 *
 * Add a transform for FILE using LENS.
 * EXCL specifies if this the file is to be included (0)
 * or excluded (1) from the LENS.
 * The LENS maybe be a module name or a full lens name.
 * If a module name is given, then lns will be the lens assumed.
 *
 * Returns:
 * 1 on success, -1 on failure
 */
int aug_transform(augeas *aug, const char *lens, const char *file, int excl);

/*
 * Function: aug_load_file
 *
 * Load a FILE using the lens that would ordinarily be used by aug_load,
 * i.e. the lens whose autoload statement matches the FILE. Similar to
 * aug_load, this function returns successfully even if FILE does not exist
 * or if the FILE can not be processed by the associated lens. It is an
 * error though if no lens can be found to process FILE. In that case, the
 * error code in AUG will be set to AUG_ENOLENS.
 *
 * Returns:
 * 0 on success, -1 on failure
 */
int aug_load_file(augeas *aug, const char *file);

/*
 * Function: aug_srun
 *
 * Run one or more newline-separated commands. The output of the commands
 * will be printed to OUT. Running just 'help' will print what commands are
 * available. Commands accepted by this are identical to what augtool
 * accepts.
 *
 * Returns:
 * the number of executed commands on success, -1 on failure, and -2 if a
 * 'quit' command was encountered
 */
int aug_srun(augeas *aug, FILE *out, const char *text);

/* Function: aug_close
 *
 * Close this Augeas instance and free any storage associated with
 * it. After running AUG_CLOSE, AUG is invalid and can not be used for any
 * more operations.
 */
void aug_close(augeas *aug);

// We can't put //* into the examples in these comments since the C
// preprocessor complains about that. So we'll resort to the equivalent but
// more wordy notation /descendant::*

/*
 * Function: aug_ns_attr
 *
 * Look up the ith node in the variable VAR and retrieve information about
 * it. Set *VALUE to the value of the node, *LABEL to its label, and
 * *FILE_PATH to the path of the file it belongs to, or to NULL if that
 * node does not belong to a file. It is permissible to pass NULL for any
 * of these variables to indicate that the caller is not interested in that
 * attribute.
 *
 * It is assumed that VAR was defined with a path expression evaluating to
 * a nodeset, like '/files/etc/hosts/descendant::*'. This function is
 * equivalent to, but faster than, aug_get(aug, "$VAR[I+1]", value),
 * respectively the corresponding calls to aug_label and aug_source. Note
 * that the index is 0-based, not 1-based.
 *
 * If VAR does not exist, or is not a nodeset, or if it has fewer than I
 * nodes, this call fails.
 *
 * The caller is responsible for freeing *FILE_PATH, but must not free
 * *VALUE or *LABEL. Those pointers are only valid up to the next call to a
 * function in this API that might modify the tree.
 *
 * Returns:
 * 1 on success (for consistency with aug_get), a negative value on failure
 */
int aug_ns_attr(const augeas* aug, const char *var, int i,
                const char **value, const char **label, char **file_path);

/*
 * Function: aug_ns_label
 *
 * Look up the LABEL and its INDEX amongst its siblings for the ith node in
 * variable VAR. (See aug_ns_attr for details of what is expected of VAR)
 *
 * Either of LABEL and INDEX may be NULL. The *INDEX will be set to the
 * number of siblings + 1 of the node $VAR[I+1] that precede it and have
 * the same label if there are at least two siblings with that label. If
 * the node $VAR[I+1] does not have any siblings with the same label as
 * itself, *INDEX will be set to 0.
 *
 * The caller must not free *LABEL. The pointer is only valid up to the
 * next call to a function in this API that might modify the tree.
 *
 * Returns:
 * 1 on success (for consistency with aug_get), a negative value on failure
 */
int aug_ns_label(const augeas *aug, const char *var, int i,
                 const char **label, int *index);

/*
 * Function: aug_ns_value
 *
 * Look up the VALUE of the ith node in variable VAR. (See aug_ns_attr for
 * details of what is expected of VAR)
 *
 * The caller must not free *VALUE. The pointer is only valid up to the
 * next call to a function in this API that might modify the tree.
 *
 * Returns:
 * 1 on success (for consistency with aug_get), a negative value on failure
 */
int aug_ns_value(const augeas *aug, const char *var, int i,
                 const char **value);

/*
 * Function: aug_ns_count
 *
 * Return the number of nodes in variable VAR. (See aug_ns_attr for details
 * of what is expected of VAR)
 *
 * Returns: the number of nodes in VAR, or a negative value on failure
 */
int aug_ns_count(const augeas *aug, const char *var);

/*
 * Function: aug_ns_count
 *
 * Put the fully qualified path to the ith node in VAR into *PATH. (See
 * aug_ns_attr for details of what is expected of VAR)
 *
 * The caller is responsible for freeing *PATH, which is allocated by this
 * function.
 *
 * Returns: 1 on success (for consistency with aug_get), a negative value
 * on failure
 */
int aug_ns_path(const augeas *aug, const char *var, int i, char **path);

/*
 * Error reporting
 */

typedef enum {
    AUG_NOERROR,        /* No error */
    AUG_ENOMEM,         /* Out of memory */
    AUG_EINTERNAL,      /* Internal error (bug) */
    AUG_EPATHX,         /* Invalid path expression */
    AUG_ENOMATCH,       /* No match for path expression */
    AUG_EMMATCH,        /* Too many matches for path expression */
    AUG_ESYNTAX,        /* Syntax error in lens file */
    AUG_ENOLENS,        /* Lens lookup failed */
    AUG_EMXFM,          /* Multiple transforms */
    AUG_ENOSPAN,        /* No span for this node */
    AUG_EMVDESC,        /* Cannot move node into its descendant */
    AUG_ECMDRUN,        /* Failed to execute command */
    AUG_EBADARG,        /* Invalid argument in function call */
    AUG_ELABEL,         /* Invalid label */
    AUG_ECPDESC         /* Cannot copy node into its descendant */
} aug_errcode_t;

/* Return the error code from the last API call */
int aug_error(augeas *aug);

/* Return a human-readable message for the error code */
const char *aug_error_message(augeas *aug);

/* Return a human-readable message elaborating the error code; might be
 * NULL. For example, when the error code is AUG_EPATHX, this will explain
 * how the path expression is invalid */
const char *aug_error_minor_message(augeas *aug);

/* Return details about the error, which might be NULL. For example, for
 * AUG_EPATHX, indicates where in the path expression the error
 * occurred. The returned value can only be used until the next API call
 */
const char *aug_error_details(augeas *aug);


#ifdef __cplusplus
}
#endif

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

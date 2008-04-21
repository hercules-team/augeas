/*
 * config.h: commonly used parameters. Most of them should eventually
 * become buildtime cobnfigurables
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

#ifndef CONFIG_H_
#define CONFIG_H_

/* The env var that points to the chroot holding files we may modify.
   Mostly useful for testing */
#define AUGEAS_ROOT_ENV "AUGEAS_ROOT"

/* The root for actual file contents */
#define AUGEAS_FILES_TREE "/files"

/* Augeas reports some information in this subtree */
#define AUGEAS_META_TREE "/augeas"

/* Information about files */
#define AUGEAS_META_FILES AUGEAS_META_TREE AUGEAS_FILES_TREE

/* The root directory */
#define AUGEAS_META_ROOT AUGEAS_META_TREE "/root"

/* How we save files. One of 'backup', 'overwrite' or 'newfile' */
#define AUGEAS_META_SAVE_MODE AUGEAS_META_TREE "/save"

/* Where the default spec files live. */
#define AUGEAS_LENS_DIR "/usr/share/augeas/lenses"

/* Name of env var that contains list of paths to search for additional
   spec files */
#define AUGEAS_LENS_ENV "AUGEAS_LENS_LIB"

/* Fairly arbitrary bound on the length of the path we
   accept from AUGEAS_SPEC_ENV */
#define MAX_ENV_SIZE 4096

/* Character separating paths in a list of paths */
#define PATH_SEP_CHAR ':'

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

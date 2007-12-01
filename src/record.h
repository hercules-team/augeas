/*
 * record.h: Support for scanning files containing records
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

#ifndef __RECORD_H
#define __RECORD_H

/*
 * A record scanner converts a file consisting of line separated records,
 * such as /etc/hosts, /etc/inittab etc. into a stream of tokens. The
 * scanner reads a file line by line and compares each line to a list of
 * record specs. If a record spec matches the line, the line is split into
 * tokens according to the spec.
 *
 * Since the scanner classifies a line according to the first spec that 
 * matches the line, it matters in which order specs are added to the scanner
 *
 */

#include "internal.h"

/* Initialize a new record scanner */
aug_rec_t aug_rec_init(const char *fsep, const char *rsep);

/* Free all memory associated with REC */
void aug_rec_free(aug_rec_t rec);

/* Add a new spec to the record scanner. If FIELDS is NULL,
   any line that matches PATTERN is added in its entirety as an inert
   token. If FIELDS is non-NULL, it must be an array of NFIELDS
   strings. For the entry with index i in the array, the field value is 
   capture group numbered i+1. If the field name starts with a '_', it 
   is translated into an inert token, otherwise into a value.
 */
int aug_rec_add_spec(aug_rec_t rec, const char *pattern, 
                     int nfields, const char **fields);

/* Parse the file NAME */
struct aug_file *aug_rec_parse(const aug_rec_t rec, const char *path,
                               const char *prefix);

/* Save AF */
int aug_rec_save(const aug_rec_t rec, struct aug_file *af);

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

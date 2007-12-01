/*
 * prov_inittab.c: /etc/inittab provider
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

#include "augeas.h"
#include "internal.h"
#include "record.h"

int augp_inittab_init(void);
int augp_inittab_load(void);
int augp_inittab_save(void);

/* All keys go underneath here */
#define INITTAB_MOUNT "/system/config/inittab"

const struct aug_provider augp_inittab = {
    .name = "inittab",
    .init = augp_inittab_init,
    .load = augp_inittab_load,
    .save = augp_inittab_save
};

/* Global variable for now */
struct aug_scanner *augp_inittab_scanner;

int augp_inittab_load(void) {
    struct aug_file *af = NULL;

    af = aug_rec_parse(augp_inittab_scanner->rec, ROOT_DIR "/etc/inittab", 
                       augp_inittab_scanner->node);
    if (af == NULL)
        return -1;
    augp_inittab_scanner->files = af;
    return 0;
}

int augp_inittab_save(void) {
    return aug_rec_save(augp_inittab_scanner->rec, augp_inittab_scanner->files);
}

int augp_inittab_init(void) {
    aug_rec_t rec;

    static const char *fields[] = 
        { "id", "_", "runlevels", "_", "action", "_", "process" };

    rec = aug_rec_init(":", "\n");
    aug_rec_add_spec(rec, "^\\s*#.*$", 0, NULL);
    aug_rec_add_spec(rec,
       "^([^:]+)(:)([^:]+)(:)([^:]+)(:)(.*)$",
        7, fields);

    augp_inittab_scanner = calloc(1, sizeof(struct aug_scanner));
    if (augp_inittab_scanner == NULL) {
        aug_rec_free(rec);
        return -1;
    }

    augp_inittab_scanner->rec = rec;
    augp_inittab_scanner->node = INITTAB_MOUNT;
    return 0;
}
/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

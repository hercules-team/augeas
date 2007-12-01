/*
 * prov_pam.c: PAM provider
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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

int augp_pam_init(void);
int augp_pam_load(void);
int augp_pam_save(void);

/* All keys go underneath here */
#define PAM_MOUNT "/system/config/pam"

const struct aug_provider augp_pam = {
    .name = "pam",
    .init = augp_pam_init,
    .load = augp_pam_load,
    .save = augp_pam_save
};

/* Global variable for now */
struct aug_scanner *augp_pam_scanner;

int augp_pam_load(void) {
    DIR *dir = NULL;
    struct dirent *e = NULL;
    static const char *dirname = ROOT_DIR "/etc/pam.d";
    char *name = NULL;
    char *prefix = NULL;
    struct aug_file *af = NULL;

    name = alloca(sizeof(dirname) + NAME_MAX + 1);
    prefix = alloca(sizeof(augp_pam_scanner->node) + 1 + NAME_MAX + 1);

    dir = opendir(dirname);
    if (dir == NULL)
        return -1;

    while ((e = readdir(dir)) != NULL) {
        if (e->d_type != DT_REG)
            continue;
        if (e->d_type == DT_LNK)
            FIXME("Links are not handled");
        
        sprintf(name, "%s/%s", dirname, e->d_name);
        sprintf(prefix, "%s/%s", augp_pam_scanner->node, e->d_name);
        af = aug_rec_parse(augp_pam_scanner->rec, name, prefix);
        if (af == NULL)
            goto error;
        if (augp_pam_scanner->files == NULL) {
            augp_pam_scanner->files = af;
        } else {
            struct aug_file *f;
            for (f = augp_pam_scanner->files;
                 f->next != NULL;
                 f = f->next);
            f->next = af;
        }
    }
    closedir(dir);

    return 0;
 error:
    if (dir)
        closedir(dir);
    if (af)
        aug_file_free(af);
    return -1;
}

int augp_pam_save(void) {
    int cnt;
    const char **children;
    struct aug_file *f;

    cnt = aug_ls(augp_pam_scanner->node, &children);
    if (cnt == -1)
        return -1;

    for (int i=0; i < cnt; i++) {
        for (f = augp_pam_scanner->files; f != NULL; f = f->next) {
            if (STREQ(children[i], f->node))
                break;
        }
        if (f == NULL) {
            printf("Save: New file for %s/%s\n", augp_pam_scanner->node, 
                   children[i]);
        } else {
            aug_rec_save(augp_pam_scanner->rec, f);
        }
    }
    
    for (f = augp_pam_scanner->files; f != NULL; f = f->next) {
        if (! aug_exists(f->node)) {
            printf("Save: File for %s deleted\n", f->node);
        }
    }

    free(children);
    return 0;
}

int augp_pam_init(void) {
    aug_rec_t rec;

    static const char *fields[] = 
        { "type", "_", "control", "_", "module", "_", "opts" };

    rec = aug_rec_init("\t", "\n");
    aug_rec_add_spec(rec, "^\\s*#.*$", 0, NULL);
    aug_rec_add_spec(rec,
       "^(\\S+)([ \\t]+)(\\[[^\\]]*\\])([ \\t]+)(\\S+)([ \\t]+)?(.*)?$",
        7, fields);
    aug_rec_add_spec(rec,
       "^(\\S+)([ \\t]+)(\\S+)([ \\t]+)(\\S+)([ \\t]+)?(.*)?$",
       7, fields);

    augp_pam_scanner = calloc(1, sizeof(struct aug_scanner));
    augp_pam_scanner->rec = rec;
    augp_pam_scanner->node = PAM_MOUNT;
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

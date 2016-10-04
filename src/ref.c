/*
 * ref.c: reference counting
 *
 * Copyright (C) 2009-2016 David Lutterkort
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

#include <config.h>

#include "ref.h"
#include <stdlib.h>

int ref_make_ref(void *ptrptr, size_t size, size_t ref_ofs) {
    *(void**) ptrptr = calloc(1, size);
    if (*(void **)ptrptr == NULL) {
        return -1;
    } else {
        void *ptr = *(void **)ptrptr;
        *((ref_t *) ((char*) ptr + ref_ofs)) = 1;
        return 0;
    }
}

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

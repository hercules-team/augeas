/*
 * list.h: Simple generic list manipulation
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

#ifndef __LIST_H
#define __LIST_H

#define list_append(head, tail)                                         \
    do {                                                                \
        if ((head) == NULL) {                                           \
            head = tail;                                                \
            break;                                                      \
        }                                                               \
        typeof(head) _p;                                                \
        for (_p = (head); _p->next != NULL; _p = _p->next);             \
        _p->next = (tail);                                              \
    } while (0)

#define list_for_each(iter, list)                                       \
    for (typeof(list) (iter) = list; (iter) != NULL; (iter) = (iter)->next)

#define list_remove(elt, list)                                          \
    do {                                                                \
        if ((elt) == (list)) {                                          \
            (list) = (elt)->next;                                       \
        } else {                                                        \
            typeof(elt) _p;                                             \
            for (_p = (list); _p != NULL && _p->next != (elt); _p = _p->next); \
            if (_p != NULL)                                             \
                _p->next = elt->next;                                   \
        }                                                               \
    } while(0)

#endif


/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

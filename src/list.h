/*
 * list.h: Simple generic list manipulation
 *
 * Copyright (C) 2007-2011 David Lutterkort
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

#ifndef LIST_H_
#define LIST_H_

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
        typeof(elt) _e = (elt);                                         \
        if (_e == (list)) {                                             \
            (list) = _e->next;                                          \
        } else {                                                        \
            typeof(_e) _p;                                              \
            for (_p = (list); _p != NULL && _p->next != _e; _p = _p->next); \
            if (_p != NULL)                                             \
                _p->next = _e->next;                                    \
        }                                                               \
        _e->next = NULL;                                                \
    } while(0)

/* Insert NEW in list LIST before element AC. NEW->next must be null,
   and ELT must really be on LIST, otherwise chaos will ensue
*/
#define list_insert_before(new, elt, list)                              \
    do {                                                                \
        if ((list) == NULL) {                                           \
            (list) = (new);                                             \
        } else if ((elt) == (list)) {                                   \
            (new)->next = (elt);                                        \
            (list) = (new);                                             \
        } else {                                                        \
            typeof(elt) _p;                                             \
            for (_p = (list); _p != NULL && _p->next != (elt); _p = _p->next); \
            if (_p != NULL) {                                           \
                (new)->next = (elt);                                    \
                _p->next = (new);                                       \
            }                                                           \
        }                                                               \
    } while(0)

#endif

#define list_free(list)                                                 \
    while ((list) != NULL) {                                            \
        typeof(list) _p = list;                                         \
        (list) = (list)->next;                                          \
        free((void *) _p);                                              \
    }

#define list_length(len, list)                                          \
    do {                                                                \
        typeof(list) _p;                                                \
        for (len=0, _p = (list); _p != NULL; len += 1, _p = _p->next);  \
    } while(0)

/* Make ELT the new head of LIST and set LIST to it */
#define list_cons(list, elt)                                            \
    do {                                                                \
        typeof(elt) _e = (elt);                                         \
        _e->next = (list);                                              \
        (list) = _e;                                                    \
    } while(0)

#define list_reverse(list)                                              \
    do {                                                                \
        typeof(list) _head = (list);                                    \
        typeof(list) _prev = NULL;                                      \
        while (_head != NULL) {                                         \
            typeof(list) _next = _head->next;                           \
            _head->next = _prev;                                        \
            _prev = _head;                                              \
            _head = _next;                                              \
        }                                                               \
        (list) = _prev;                                                 \
    } while (0)

/* Append ELT to the end of LIST. TAIL must be NULL or a pointer to
   the last element of LIST. ELT may also be a list
*/
#define list_tail_cons(list, tail, elt)                                 \
    do {                                                                \
        /* Append ELT at the end of LIST */                             \
        if ((list) == NULL) {                                           \
            (list) = (elt);                                             \
        } else {                                                        \
            if ((tail) == NULL)                                         \
                for ((tail) = (list); (tail)->next != NULL;             \
                     (tail) = (tail)->next);                            \
            (tail)->next = (elt);                                       \
        }                                                               \
        /* Make sure TAIL is the last element on the combined LIST */   \
        (tail) = (elt);                                                 \
        if ((tail) != NULL)                                             \
            while ((tail)->next != NULL)                                \
                (tail) = (tail)->next;                                  \
    } while(0)

/*
 * Local variables:
 *  indent-tabs-mode: nil
 *  c-indent-level: 4
 *  c-basic-offset: 4
 *  tab-width: 4
 * End:
 */

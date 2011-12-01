/*
 * memory.c: safer memory allocation
 *
 * Copyright (C) 2008-2011 Daniel P. Berrange
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
 */


#ifndef MEMORY_H_
#define MEMORY_H_

#include "internal.h"

/* Don't call these directly - use the macros below */
int mem_alloc_n(void *ptrptr, size_t size, size_t count) ATTRIBUTE_RETURN_CHECK;
int mem_realloc_n(void *ptrptr, size_t size, size_t count) ATTRIBUTE_RETURN_CHECK;


/**
 * ALLOC:
 * @ptr: pointer to hold address of allocated memory
 *
 * Allocate sizeof(*ptr) bytes of memory and store
 * the address of allocated memory in 'ptr'. Fill the
 * newly allocated memory with zeros.
 *
 * Returns -1 on failure, 0 on success
 */
#define ALLOC(ptr) mem_alloc_n(&(ptr), sizeof(*(ptr)), 1)

/**
 * ALLOC_N:
 * @ptr: pointer to hold address of allocated memory
 * @count: number of elements to allocate
 *
 * Allocate an array of 'count' elements, each sizeof(*ptr)
 * bytes long and store the address of allocated memory in
 * 'ptr'. Fill the newly allocated memory with zeros.
 *
 * Returns -1 on failure, 0 on success
 */
#define ALLOC_N(ptr, count) mem_alloc_n(&(ptr), sizeof(*(ptr)), (count))

/**
 * REALLOC_N:
 * @ptr: pointer to hold address of allocated memory
 * @count: number of elements to allocate
 *
 * Re-allocate an array of 'count' elements, each sizeof(*ptr)
 * bytes long and store the address of allocated memory in
 * 'ptr'. Fill the newly allocated memory with zeros
 *
 * Returns -1 on failure, 0 on success
 */
#define REALLOC_N(ptr, count) mem_realloc_n(&(ptr), sizeof(*(ptr)), (count))

/**
 * FREE:
 * @ptr: pointer holding address to be freed
 *
 * Free the memory stored in 'ptr' and update to point
 * to NULL.
 */
#define FREE(ptr)                               \
  do {                                          \
    free(ptr);                                  \
    (ptr) = NULL;                               \
  } while(0)

#endif /* __VIR_MEMORY_H_ */

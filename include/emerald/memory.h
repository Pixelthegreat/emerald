/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Shim to keep track of memory allocations
 */
#ifndef EMERALD_MEMORY_H
#define EMERALD_MEMORY_H

#include <emerald/core.h>

EM_API em_bool_t em_track_allocations;
EM_API size_t em_memory_usage; /* valid only with allocation tracking */

/* functions */
EM_API void *em_allocate(size_t size, const char *file, em_ssize_t line); /* allocate memory */
EM_API void *em_realloc(void *p, size_t size); /* reallocate memory */
EM_API void em_free(void *p); /* free memory */
EM_API void em_print_allocs(void); /* log allocation info */

#define em_malloc(size) em_allocate(size, __FILE__, __LINE__)

#endif /* EMERALD_MEMORY_H */

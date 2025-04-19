/*
 * Shim to keep track of memory allocations
 */
#ifndef EMERALD_MEMORY_H
#define EMERALD_MEMORY_H

#include <emerald/core.h>

/* functions */
EM_API void *em_allocate(size_t size, const char *file, em_ssize_t line); /* allocate memory */
EM_API void em_free(void *p); /* free memory */
EM_API void em_print_allocs(void); /* log allocation info */

#define em_malloc(size) em_allocate(size, __FILE__, __LINE__)

#endif /* EMERALD_MEMORY_H */

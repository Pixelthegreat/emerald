/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * System for tracking object reference counts
 */
#ifndef EMERALD_REFOBJ_H
#define EMERALD_REFOBJ_H

#include <emerald/core.h>

struct em_refobj;

/* clean up modes */
typedef enum em_cleanup_mode {
	EM_CLEANUP_MODE_IMMEDIATE = 0,
	EM_CLEANUP_MODE_WAITLIST,

	EM_CLEANUP_MODE_COUNT,
} em_cleanup_mode_t;

/* reference object list */
typedef struct em_reflist {
	em_bool_t init; /* init flag */
	int nlock; /* number of destruction locks held */
	struct {
		struct em_refobj *first; /* first item in list */
		struct em_refobj *last; /* last item in list */
	} normal, wait;
} em_reflist_t;

#define EM_REFLIST_INIT ((em_reflist_t){EM_FALSE})

/* reference object */
typedef struct em_refobj {
	int refcnt; /* reference count */
	em_cleanup_mode_t mode; /* how to handle destruction when refcnt is zero */
	void (*free)(void *); /* destructor */
	em_reflist_t *list; /* reference list */
	struct em_refobj *prev; /* previous sibling */
	struct em_refobj *next; /* next sibling */
} em_refobj_t;

#define EM_REFOBJ(p) ((em_refobj_t *)(p))

/* functions */
EM_API em_result_t em_reflist_init(em_reflist_t *list); /* initialize list */
EM_API void em_reflist_add(em_reflist_t *list, em_refobj_t *obj); /* add item to list */
EM_API void em_reflist_move(em_reflist_t *list, em_refobj_t *obj); /* move item to destruction wait list */
EM_API void em_reflist_move_back(em_reflist_t *list, em_refobj_t *obj); /* move item to normal list */
EM_API void em_reflist_remove(em_reflist_t *list, em_refobj_t *obj); /* remove item from list */
EM_API void em_reflist_cleanup(em_reflist_t *list); /* remove items from destruction wait list */
EM_API void em_reflist_lock(em_reflist_t *list); /* lock list to prevent any immediate item destruction */
EM_API void em_reflist_unlock(em_reflist_t *list); /* unlock list */
EM_API void em_reflist_destroy(em_reflist_t *list); /* destroy list */

EM_API em_refobj_t *em_refobj_new_full(em_reflist_t *list, size_t size, em_cleanup_mode_t mode, const char *file, em_ssize_t line); /* create reference object */
EM_API em_refobj_t *em_refobj_incref(em_refobj_t *obj); /* increase reference count */
EM_API void em_refobj_decref(em_refobj_t *obj); /* decrease reference count */
EM_API void em_refobj_decref_no_free(em_refobj_t *obj); /* decrease reference count without freeing */
EM_API void em_refobj_free(em_refobj_t *obj); /* free reference object */
EM_API void em_refobj_free_bare(em_refobj_t *obj); /* free without calling handler */

#define em_refobj_new(list, size, mode) em_refobj_new_full(list, size, mode, __FILE__, __LINE__)

#endif /* EMERALD_REFOBJ_H */

/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/log.h>
#include <emerald/utf8.h>
#include <emerald/hash.h>
#include <emerald/memory.h>

#ifdef DEBUG
em_bool_t em_track_allocations = EM_TRUE;
#else
em_bool_t em_track_allocations = EM_FALSE;
#endif

static size_t nalloc; /* current number of allocations */
static size_t ntotal; /* total number of allocations */

/* per file memory tracking linked list */
struct mlist;

struct mblk {
	struct mlist *list; /* associated list */
	em_ssize_t line; /* associated line of code */
	size_t size; /* allocation size */
	struct mblk *prev; /* previous block */
	struct mblk *next; /* next block */
};

struct mlist {
	struct mblk *first; /* first allocation */
	struct mblk *last; /* last allocation */
	struct mlist *next; /* next list */
	em_hash_t hash; /* file path hash */
	char path[]; /* file path */
};

static struct mlist *first = NULL;
static struct mlist *last = NULL;

/* track allocation */
static void *track_alloc(size_t size, const char *file, em_ssize_t line) {

	em_hash_t hash = em_utf8_strhash(file);

	/* find or create allocation list */
	struct mlist *list = first;
	while (list) {

		if (list->hash == hash) break;
		list = list->next;
	}
	if (!list) {

		size_t len = strlen(file);
		list = (struct mlist *)malloc(sizeof(struct mlist) + len+1);
		if (!list) return NULL;

		list->first = NULL;
		list->last = NULL;
		list->next = NULL;
		list->hash = hash;
		memcpy(list->path, file, len);
		list->path[len] = 0;
		
		if (!first) first = list;
		if (last) last->next = list;
		last = list;
	}

	/* allocate block */
	struct mblk *blk = (struct mblk *)malloc(sizeof(struct mblk) + size);
	if (!blk) return NULL;

	blk->list = list;
	blk->line = line;
	blk->next = NULL;
	blk->size = size;

	blk->prev = list->last;
	if (!list->first) list->first = blk;
	if (list->last) list->last->next = blk;
	list->last = blk;

	return (void *)(blk + 1);
}

/* track reallocation; necessary to readjust the allocation list */
static void *track_realloc(void *p, size_t size) {

	if (!p) return NULL;

	struct mblk *blk = (struct mblk *)p - 1;
	struct mblk *oblk = blk;

	blk = realloc(blk, sizeof(struct mblk) + size);
	if (!blk) return NULL;

	blk->size = size;

	/* update links */
	if (blk->prev) blk->prev->next = blk;
	if (blk->next) blk->next->prev = blk;
	if (blk->list->first == oblk) blk->list->first = blk;
	if (blk->list->last == oblk) blk->list->last = blk;

	return (void *)(blk + 1);
}

/* track free */
static void track_free(void *p) {

	if (!p) return;

	struct mblk *blk = (struct mblk *)p - 1;

	if (blk->prev) blk->prev->next = blk->next;
	if (blk->next) blk->next->prev = blk->prev;
	if (blk->list->first == blk) blk->list->first = blk->next;
	if (blk->list->last == blk) blk->list->last = blk->prev;

	free(blk);
}

/* allocate memory */
EM_API void *em_allocate(size_t size, const char *file, em_ssize_t line) {

	void *p = em_track_allocations? track_alloc(size, file, line): malloc(size);
	if (!p) {

		em_log_fatal("Allocation of %zu bytes failed", size);
		return NULL;
	}

	nalloc++;
	ntotal++;
	return p;
}

/* reallocate memory */
EM_API void *em_realloc(void *p, size_t size) {

	if (!p) return NULL;

	return em_track_allocations? track_realloc(p, size): realloc(p, size);
}

/* free memory */
EM_API void em_free(void *p) {

	if (!p) return;

	if (!em_track_allocations) free(p);
	else track_free(p);
	nalloc--;
}

/* log allocation info */
EM_API void em_print_allocs(void) {

	em_log_info("%zu allocations, %zu frees", ntotal, ntotal-nalloc);

	/* free allocation lists and what not */
	struct mlist *list = first;
	while (list) {

		struct mlist *next = list->next;

		struct mblk *blk = list->first;
		while (blk) {

			struct mblk *next = blk->next;

			em_log_warning("Unresolved allocation in file '%s' at line %ld (%zu bytes)", list->path, blk->line, blk->size);

			free(blk);
			blk = next;
		}

		free(list);
		list = next;
	}
}

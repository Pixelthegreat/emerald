/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_MAP_H
#define EMERALD_MAP_H

#include <emerald/core.h>
#include <emerald/object.h>

/* map entry */
typedef struct em_map_entry {
	em_hash_t key; /* hashed key */
	em_value_t value; /* item value */
	struct em_map_entry *previous; /* previous entry */
	struct em_map_entry *next; /* next entry */
} em_map_entry_t;

/* map */
typedef struct em_map {
	em_object_t base;
	em_map_entry_t *first; /* first entry */
	em_map_entry_t *last; /* last entry */
} em_map_t;

#define EM_MAP(p) ((em_map_t *)(p))

/* functions */
EM_API em_value_t em_map_new(void); /* create map */
EM_API void em_map_set(em_value_t object, em_hash_t key, em_value_t value); /* set value */
EM_API em_value_t em_map_get(em_value_t object, em_hash_t key); /* get value */

#endif /* EMERALD_MAP_H */

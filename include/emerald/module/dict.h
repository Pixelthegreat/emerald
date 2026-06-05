/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_MODULE_DICT_H
#define EMERALD_MODULE_DICT_H

#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/object.h>
#include <emerald/module.h>

EM_API em_module_t em_module_dict;

/* dictionary */
typedef struct em_dict {
	em_object_t base;
	size_t count; /* number of items */
	em_map_entry_t *first; /* first entry */
	em_map_entry_t *last; /* last entry */
} em_dict_t;

#define EM_DICT(p) ((em_dict_t *)(p))

/* dictionary item */
typedef struct em_dict_item {
	em_object_t base;
	size_t index; /* index of item */
	em_value_t key; /* item key */
	em_value_t value; /* item value */
} em_dict_item_t;

#define EM_DICT_ITEM(p) ((em_dict_item_t *)(p))

/* dictionary iterator */
typedef struct em_dict_iterator {
	em_object_t base;
	size_t count; /* number of items to iterate */
	em_value_t dict_item; /* temporary item object */
	struct {
		em_value_t key; /* item key */
		em_value_t value; /* item value */
	} items[]; /* items to iterate */
} em_dict_iterator_t;

#define EM_DICT_ITERATOR(p) ((em_dict_iterator_t *)(p))

/* functions */
EM_API em_value_t em_dict_new(em_value_t map); /* create dictionary */
EM_API void em_dict_set(em_value_t object, em_value_t key, em_hash_t key_hash, em_value_t value); /* set value */
EM_API em_value_t em_dict_get(em_value_t object, em_hash_t key_hash); /* get value */
EM_API em_bool_t em_is_dict(em_value_t v); /* determine if value is dictionary */

EM_API em_value_t em_dict_item_new(void); /* create temporary dictionary item */
EM_API void em_dict_item_set(em_value_t object, size_t index, em_value_t key, em_value_t value); /* set dictionary item */
EM_API em_value_t em_dict_item_get_key(em_value_t object); /* get dictionary item key */
EM_API em_value_t em_dict_item_get_value(em_value_t object); /* get dictionary item value */
EM_API em_bool_t em_is_dict_item(em_value_t v); /* determine if value is dictionary item */

EM_API em_value_t em_dict_iterator_new(em_value_t dict); /* create dictionary iterator */
EM_API size_t em_dict_iterator_get_length(em_value_t object); /* get length of iterator */
EM_API em_value_t em_dict_iterator_get(em_value_t object, size_t index); /* get iterator item */

#endif /* EMERALD_MODULE_DICT_H */

/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <emerald/log.h>
#include <emerald/hash.h>
#include <emerald/memory.h>
#include <emerald/string.h>
#include <emerald/context.h>
#include <emerald/map.h>

/* object type */
static em_value_t get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos);
static em_value_t get_by_index(em_value_t v, em_value_t i, em_pos_t *pos);
static em_result_t set_by_hash(em_value_t a, em_hash_t hash, em_value_t b, em_pos_t *pos);
static em_result_t set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos);
static em_value_t call(em_context_t *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos);
static em_value_t to_string(em_value_t v, em_pos_t *pos);

em_object_type_t type = {
	.get_by_hash = get_by_hash,
	.get_by_index = get_by_index,
	.set_by_hash = set_by_hash,
	.set_by_index = set_by_index,
	.call = call,
	.to_string = to_string,
};

/* get value by key hash */
static em_value_t get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos) {

	return em_map_get(v, hash);
}

/* get value by index */
static em_value_t get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	em_hash_t hash = em_value_hash(i, pos);
	return em_map_get(v, hash);
}

/* set value by key hash */
static em_result_t set_by_hash(em_value_t a, em_hash_t hash, em_value_t b, em_pos_t *pos) {

	em_map_set(a, hash, b);
	return EM_RESULT_SUCCESS;
}

/* set value by index */
static em_result_t set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos) {

	em_hash_t hash = em_value_hash(i, pos);
	em_map_set(a, hash, b);
	return EM_RESULT_SUCCESS;
}

/* call map */
static em_value_t call(em_context_t *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value = em_map_get(v, em_utf8_strhash("_call"));
	if (!EM_VALUE_OK(value)) {

		em_log_runtime_error(pos, "Invalid operation");
		return EM_VALUE_FAIL;
	}
	return em_value_call(context, value, args, nargs, pos);
}

/* get string representation of map */
static em_value_t to_string(em_value_t v, em_pos_t *pos) {

	em_value_t value = em_map_get(v, em_utf8_strhash("_toString"));
	if (!EM_VALUE_OK(value))
		return em_string_new_from_utf8("{...}", 5);

	em_value_t args[1] = {};
	return em_value_call(pos->context, value, args, 0, pos);
}

/* free map */
static void map_free(void *p) {

	em_map_t *map = EM_MAP(p);

	em_map_entry_t *entry = map->first;
	while (entry) {

		em_map_entry_t *next = entry->next;

		em_value_decref(entry->value);
		em_free(entry);

		entry = next;
	}
}

/* create map */
EM_API em_value_t em_map_new(void) {

	em_value_t value = em_object_new(&type, sizeof(em_map_t));
	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(map)->free = map_free;

	map->first = NULL;
	map->last = NULL;
	map->userdata = NULL;

	return value;
}

/* set value */
EM_API void em_map_set(em_value_t object, em_hash_t key, em_value_t value) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = map->first;
	while (entry) {

		if (entry->key == key)
			break;
		entry = entry->next;
	}
	if (!entry) {

		entry = em_malloc(sizeof(em_map_entry_t));

		entry->key = key;
		entry->value = EM_VALUE_FAIL;
		entry->previous = map->last;
		entry->next = NULL;
		if (!map->first) map->first = entry;
		if (map->last) map->last->next = entry;
		map->last = entry;
	}

	if (em_value_is(entry->value, value))
		return;

	if (EM_VALUE_OK(entry->value)) em_value_decref(entry->value);
	entry->value = value;
	em_value_incref(value);
}

/* get value */
EM_API em_value_t em_map_get(em_value_t object, em_hash_t key) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = map->first;
	while (entry) {

		if (entry->key == key)
			return entry->value;
		entry = entry->next;
	}
	return EM_VALUE_FAIL;
}

/* determine if value is map */
EM_API em_bool_t em_is_map(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT &&
	       EM_OBJECT_FROM_VALUE(v)->type == &type;
}

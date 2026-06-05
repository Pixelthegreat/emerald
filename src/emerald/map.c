/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static em_object_type_t type = {
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
	em_map_set_key(a, i, hash, b);
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

		em_value_decref(entry->key);
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

/* set value with key */
EM_API void em_map_set_key(em_value_t object, em_value_t key, em_hash_t key_hash, em_value_t value) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = map->first;
	while (entry) {

		if (entry->key_hash == key_hash || !EM_VALUE_OK(entry->value))
			break;
		entry = entry->next;
	}

	/* create entry */
	if (!entry) {

		entry = em_malloc(sizeof(em_map_entry_t));
		memset(entry, 0, sizeof(em_map_entry_t));

		entry->key = EM_VALUE_FAIL;
		entry->key_hash = key_hash;
		entry->value = EM_VALUE_FAIL;
		entry->previous = map->last;
		entry->next = NULL;
		if (!map->first) map->first = entry;
		if (map->last) map->last->next = entry;
		map->last = entry;
	}

	if (em_value_is(entry->value, value))
		return;

	/* set values */
	entry->key_hash = key_hash;
	em_value_decref(entry->value);
	entry->value = value;
	em_value_incref(value);

	if (!em_value_is(entry->key, key)) {

		em_value_decref(entry->key);
		entry->key = key;
		em_value_incref(key);
	}
}

/* set value with key hash */
EM_API void em_map_set(em_value_t object, em_hash_t key, em_value_t value) {

	em_map_set_key(object, EM_VALUE_FAIL, key, value);
}

/* get value */
EM_API em_value_t em_map_get(em_value_t object, em_hash_t key) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = map->first;
	while (entry) {

		if (entry->key_hash == key && EM_VALUE_OK(entry->value))
			break;
		entry = entry->next;
	}
	if (!entry) return EM_VALUE_FAIL;

	/* move entry to front */
	if (entry != map->first) {

		if (entry == map->last)
			map->last = entry->previous;
		if (entry->previous) entry->previous->next = entry->next;
		if (entry->next) entry->next->previous = entry->previous;

		entry->previous = NULL;
		entry->next = map->first;
		if (!map->last) map->last = entry;
		if (map->first) map->first->previous = entry;
		map->first = entry;
	}
	return entry->value;
}

/* reset map without freeing all of its resources */
EM_API void em_map_soft_reset(em_value_t object) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = map->first;
	while (entry) {

		em_value_decref(entry->key);
		em_value_decref(entry->value);
		entry->key = EM_VALUE_FAIL;
		entry->key_hash = 0;
		entry->value = EM_VALUE_FAIL;
		entry = entry->next;
	}
}

/* copy map */
EM_API em_value_t em_map_copy(em_value_t object) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_value_t new = em_map_new();

	em_map_entry_t *entry = map->first;
	while (entry) {

		em_map_set_key(new, entry->key, entry->key_hash, entry->value);
		entry = entry->next;
	}
	return new;
}

/* determine if value is map */
EM_API em_bool_t em_is_map(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT &&
	       EM_OBJECT_FROM_VALUE(v)->type == &type;
}

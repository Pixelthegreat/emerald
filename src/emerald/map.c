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

/* print layout (debug) */
static void print_layout(em_map_t *map) {

	printf("layout:");
	for (size_t i = 0; i < map->capacity; i++) {

		if (i % 4 == 0) fputc('\n', stdout);
		else fputs("  ", stdout);

		printf(
			"%04zx: \e[90m%08x\e[39m",
			i, map->entries[i].key_hash
		);
	}
	fputc('\n', stdout);
}

/* grow map capacity */
static em_map_entry_t *create_entry(em_map_t *map, em_hash_t hash, em_bool_t regrow);

static void grow_map(em_map_t *map) {

	em_map_entry_t *old_entries = map->entries;
	size_t old_capacity = map->capacity;

	if (!map->capacity)
		map->capacity = 16;
	else
		map->capacity *= 2;
	map->count = 0;
	map->load_factor = (map->capacity >> 3) * 5;

	size_t size = sizeof(em_map_entry_t) * map->capacity;

	map->entries = (em_map_entry_t *)malloc(size);
	memset(map->entries, 0, size);

	/* copy old entries */
	if (!old_entries) return;

	for (size_t i = 0; i < old_capacity; i++) {

		em_map_entry_t *old = old_entries+i;
		em_map_entry_t *new = create_entry(
			map, old->key_hash, EM_FALSE
		);
		new->key = old->key;
		new->key_hash = old->key_hash;
		new->value = old->value;
	}
	free(old_entries);
}

/* get or create entry */
static em_map_entry_t *create_entry(em_map_t *map, em_hash_t hash, em_bool_t regrow) {

	if (regrow && map->count >= map->load_factor)
		grow_map(map);

	/* check remaining keys */
	size_t index = (size_t)hash & (map->capacity - 1);
	size_t start = index;
	do {
		if (map->entries[index].key_hash == hash)
			return map->entries+index;

		else if (!map->entries[index].key_hash) {

			map->entries[index].key_hash = hash;
			map->count++;
			return map->entries+index;
		}
		index = (index + 1) & (map->capacity - 1);

	} while (index != start);
	return NULL;
}

/* get entry */
static em_map_entry_t *get_entry(em_map_t *map, em_hash_t hash) {

	if (!map->capacity)
		return NULL;

	size_t index = (size_t)hash & (map->capacity - 1);
	size_t start = index;
	do {
		if (map->entries[index].key_hash == hash &&
		    EM_VALUE_OK(map->entries[index].value))
			return map->entries+index;
		else if (!map->entries[index].key_hash)
			return NULL;

		index = (index + 1) & (map->capacity - 1);

	} while (index != start);
	return NULL;
}

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

	for (size_t i = 0; i < map->capacity; i++) {

		em_map_entry_t *entry = map->entries+i;
		if (!entry->key_hash)
			continue;

		em_value_decref(entry->key);
		em_value_decref(entry->value);
	}
	if (map->entries)
		free(map->entries);
}

/* create map */
EM_API em_value_t em_map_new(void) {

	em_value_t value = em_object_new(&type, sizeof(em_map_t));
	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(map)->free = map_free;

	map->entries = NULL;
	map->count = 0;
	map->capacity = 0;
	map->load_factor = 0;

	return value;
}

/* set value with key */
EM_API void em_map_set_key(em_value_t object, em_value_t key, em_hash_t key_hash, em_value_t value) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = create_entry(map, key_hash, EM_TRUE);

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

	em_map_entry_t *entry = get_entry(map, key);
	if (!entry) return EM_VALUE_FAIL;

	return entry->value;
}

/* reset map without freeing all of its resources */
EM_API void em_map_soft_reset(em_value_t object) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	for (size_t i = 0; i < map->capacity; i++) {

		em_map_entry_t *entry = map->entries+i;
		if (!entry->key_hash)
			continue;

		em_value_decref(entry->key);
		em_value_decref(entry->value);
		entry->key = EM_VALUE_FAIL;
		entry->key_hash = 0;
		entry->value = EM_VALUE_FAIL;
	}
	map->count = 0;
}

/* copy map */
EM_API em_value_t em_map_copy(em_value_t object) {

	em_map_t *map = EM_MAP(EM_OBJECT_FROM_VALUE(object));

	em_value_t new = em_map_new();

	for (size_t i = 0; i < map->capacity; i++) {

		em_map_entry_t *entry = map->entries+i;
		if (!entry->key_hash)
			continue;
		em_map_set_key(new, entry->key, entry->key_hash, entry->value);
	}
	return new;
}

/* determine if value is map */
EM_API em_bool_t em_is_map(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT &&
	       EM_OBJECT_FROM_VALUE(v)->type == &type;
}

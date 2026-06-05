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
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/string.h>
#include <emerald/context.h>
#include <emerald/module/dict.h>

/* dict module */
static em_result_t initialize(em_context_t *context, em_value_t map);

em_module_t em_module_dict = {
	.initialize = initialize,
};

/* create dictionary */
static em_value_t dict_Dict(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t map = em_none;

	if (nargs && em_util_parse_args(pos, args, nargs, "m", &map) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_dict_new(map);
}

/* create dictionary iterator */
static em_value_t dict_iterate(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t dict;

	if (em_util_parse_args(pos, args, nargs, "M", &dict) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	return em_dict_iterator_new(dict);
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_dict", mod);

	em_util_set_function(mod, "Dict", dict_Dict);
	em_util_set_function(mod, "iterate", dict_iterate);

	return EM_RESULT_SUCCESS;
}

/* object type */
static em_value_t dict_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos);
static em_result_t dict_set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos);
static em_value_t dict_to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t dict_type = {
	.get_by_index = dict_get_by_index,
	.set_by_index = dict_set_by_index,
	.to_string = dict_to_string,
};

/* get value by index */
static em_value_t dict_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	em_hash_t hash = em_value_hash(i, pos);
	return em_dict_get(v, hash);
}

/* set value by index */
static em_result_t dict_set_by_index(em_value_t a, em_value_t i, em_value_t b, em_pos_t *pos) {

	em_hash_t hash = em_value_hash(i, pos);
	em_dict_set(a, i, hash, b);
	return EM_RESULT_SUCCESS;
}

/* get string representation */
static em_value_t dict_to_string(em_value_t v, em_pos_t *pos) {

	return em_string_new_from_utf8("Dict({...})", 11);
}

/* free dictionary */
static void dict_free(void *p) {

	em_dict_t *dict = EM_DICT(p);

	em_map_entry_t *entry = dict->first;
	while (entry) {

		em_map_entry_t *next = entry->next;

		em_value_decref(entry->key);
		em_value_decref(entry->value);
		em_free(entry);

		entry = next;
	}
}

/* create dictionary */
EM_API em_value_t em_dict_new(em_value_t map) {

	em_value_t value = em_object_new(&dict_type, sizeof(em_dict_t));
	em_dict_t *dict = EM_DICT(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(dict)->free = dict_free;

	dict->count = 0;
	dict->first = NULL;
	dict->last = NULL;

	/* copy values from map */
	if (!em_is_map(map))
		return value;

	em_map_t *p_map = EM_MAP(EM_OBJECT_FROM_VALUE(map));
	em_map_entry_t *entry = p_map->first;

	while (entry) {

		if (EM_VALUE_OK(entry->key))
			em_dict_set(value, entry->key, entry->key_hash, entry->value);
		entry = entry->next;
	}
	return value;
}

/* set value */
EM_API void em_dict_set(em_value_t object, em_value_t key, em_hash_t key_hash, em_value_t value) {

	em_dict_t *dict = EM_DICT(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = dict->first;
	while (entry) {

		if (entry->key_hash == key_hash)
			break;
		entry = entry->next;
	}

	/* create entry */
	if (!entry) {

		entry = em_malloc(sizeof(em_map_entry_t));
		memset(entry, 0, sizeof(em_map_entry_t));

		entry->key = EM_VALUE_FAIL;
		entry->value = EM_VALUE_FAIL;
		entry->previous = dict->last;
		entry->next = NULL;
		if (!dict->first) dict->first = entry;
		if (dict->last) dict->last->next = entry;
		dict->last = entry;

		dict->count++;
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

/* get value */
EM_API em_value_t em_dict_get(em_value_t object, em_hash_t key_hash) {

	em_dict_t *dict = EM_DICT(EM_OBJECT_FROM_VALUE(object));

	em_map_entry_t *entry = dict->first;
	while (entry) {

		if (entry->key_hash == key_hash)
			return entry->value;
		entry = entry->next;
	}
	return EM_VALUE_FAIL;
}

/* determine if value is dictionary */
EM_API em_bool_t em_is_dict(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &dict_type;
}

/* dictionary item type */
static em_value_t item_get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos);

static em_object_type_t item_type = {
	.get_by_hash = item_get_by_hash,
};

/* get value by hash */
static em_value_t item_get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos) {

	em_dict_item_t *item = EM_DICT_ITEM(EM_OBJECT_FROM_VALUE(v));

	/* 'index' */
	if (hash == 0x5fb28d2)
		return EM_VALUE_INT((em_inttype_t)item->index);
	/* 'key' */
	else if (hash == 0x19e5f)
		return item->key;
	/* 'value' */
	else if (hash == 0x6ac9171)
		return item->value;

	return EM_VALUE_FAIL;
}

/* free dictionary item */
static void item_free(void *p) {

	em_dict_item_t *item = EM_DICT_ITEM(p);

	em_value_decref(item->key);
	em_value_decref(item->value);
}

/* create temporary dictionary item */
EM_API em_value_t em_dict_item_new(void) {

	em_value_t value = em_object_new(&item_type, sizeof(em_dict_item_t));
	em_dict_item_t *item = EM_DICT_ITEM(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(item)->free = item_free;

	item->index = 0;
	item->key = EM_VALUE_FAIL;
	item->value = EM_VALUE_FAIL;

	return value;
}

/* set dictionary item */
EM_API void em_dict_item_set(em_value_t object, size_t index, em_value_t key, em_value_t value) {

	em_dict_item_t *item = EM_DICT_ITEM(EM_OBJECT_FROM_VALUE(object));

	item->index = index;

	if (!em_value_is(item->key, key)) {

		em_value_decref(item->key);
		item->key = key;
		em_value_incref(key);
	}
	if (!em_value_is(item->value, value)) {

		em_value_decref(item->value);
		item->value = value;
		em_value_incref(value);
	}
}

/* get dictionary item key */
EM_API em_value_t em_dict_item_get_key(em_value_t object) {

	return EM_DICT_ITEM(EM_OBJECT_FROM_VALUE(object))->key;
}

/* get dictionary item value */
EM_API em_value_t em_dict_item_get_value(em_value_t object) {

	return EM_DICT_ITEM(EM_OBJECT_FROM_VALUE(object))->value;
}

/* determine if value is dictionary item */
EM_API em_bool_t em_is_dict_item(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &item_type;
}

/* dictionary iterator type */
static em_value_t iterator_get_by_index(em_value_t a, em_value_t i, em_pos_t *pos);
static em_value_t iterator_length_of(em_value_t v, em_pos_t *pos);

static em_object_type_t iterator_type = {
	.get_by_index = iterator_get_by_index,
	.length_of = iterator_length_of,
};

/* get value by index */
static em_value_t iterator_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	em_dict_iterator_t *iterator = EM_DICT_ITERATOR(EM_OBJECT_FROM_VALUE(v));

	if (i.type != EM_VALUE_TYPE_INT ||
	    i.value.te_inttype < 0 ||
	    i.value.te_inttype >= (em_inttype_t)iterator->count)
		return EM_VALUE_FAIL;

	return em_dict_iterator_get(v, (size_t)i.value.te_inttype);
}

/* get length of iterator */
static em_value_t iterator_length_of(em_value_t v, em_pos_t *pos) {

	em_dict_iterator_t *iterator = EM_DICT_ITERATOR(EM_OBJECT_FROM_VALUE(v));
	return EM_VALUE_INT((em_inttype_t)iterator->count);
}

/* free dictionary iterator */
static void iterator_free(void *p) {

	em_dict_iterator_t *iterator = EM_DICT_ITERATOR(p);

	for (size_t i = 0; i < iterator->count; i++) {

		em_value_decref(iterator->items[i].key);
		em_value_decref(iterator->items[i].value);
	}
	em_value_decref(iterator->dict_item);
}

/* create dictionary iterator */
EM_API em_value_t em_dict_iterator_new(em_value_t dict) {

	em_dict_t *p_dict = EM_DICT(EM_OBJECT_FROM_VALUE(dict));
	em_dict_iterator_t *iterator = NULL;

	size_t full_size = sizeof(iterator->items[0]) * p_dict->count;

	em_value_t value = em_object_new(&iterator_type, sizeof(em_dict_iterator_t) + full_size);
	iterator = EM_DICT_ITERATOR(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(iterator)->free = iterator_free;

	iterator->count = p_dict->count;
	iterator->dict_item = em_dict_item_new();
	em_value_incref(iterator->dict_item);

	/* copy key and value pairs */
	size_t index = 0;
	em_map_entry_t *entry = p_dict->first;

	while (entry) {

		iterator->items[index].key = entry->key;
		iterator->items[index].value = entry->value;

		em_value_incref(entry->key);
		em_value_incref(entry->value);

		entry = entry->next;
		index++;
	}
	return value;
}

/* get length of iterator */
EM_API size_t em_dict_iterator_get_length(em_value_t object) {

	em_dict_iterator_t *iterator = EM_DICT_ITERATOR(EM_OBJECT_FROM_VALUE(object));
	return iterator->count;
}

/* get iterator item */
EM_API em_value_t em_dict_iterator_get(em_value_t object, size_t index) {

	em_dict_iterator_t *iterator = EM_DICT_ITERATOR(EM_OBJECT_FROM_VALUE(object));

	em_dict_item_set(
			iterator->dict_item,
			index,
			iterator->items[index].key,
			iterator->items[index].value
	);
	return iterator->dict_item;
}

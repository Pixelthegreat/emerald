/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald/core.h>
#include <emerald/context.h>
#include <emerald/string.h>
#include <emerald/function.h>
#include <emerald/util.h>
#include <emerald/utf8.h>
#include <emerald/class.h>

/* bound method type */
static em_value_t method_call(em_context_t *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos);
static em_value_t method_to_string(em_value_t v, em_pos_t *pos);
static void method_free(void *p);

static em_object_type_t method_type = {
	.call = method_call,
	.to_string = method_to_string,
};

/* class type */
static em_value_t class_call(em_context_t *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos);
static em_value_t class_get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos);
static em_value_t class_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos);
static em_value_t class_to_string(em_value_t v, em_pos_t *pos);
static void class_free(void *p);

static em_object_type_t class_type = {
	.call = class_call,
	.get_by_hash = class_get_by_hash,
	.get_by_index = class_get_by_index,
	.to_string = class_to_string,
};

/* call method */
static em_value_t method_call(em_context_t *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_method_t *method = EM_METHOD(EM_OBJECT_FROM_VALUE(v));

	em_value_t newargs[EM_FUNCTION_MAX_ARGUMENTS+1] = {method->binding};
	memcpy(newargs+1, args, nargs * sizeof(em_value_t));

	em_value_incref(method->binding);
	em_value_t result = em_value_call(context, method->function, newargs, nargs+1, pos);
	em_value_decref_no_free(method->binding);

	return result;
}

/* get string representation */
static em_value_t method_to_string(em_value_t v, em_pos_t *pos) {

	return em_value_to_string(EM_METHOD(EM_OBJECT_FROM_VALUE(v))->binding, pos);
}

/* destroy method */
static void method_free(void *p) {

	em_method_t *method = EM_METHOD(p);
	em_value_decref(method->function);
}

/* copy values */
static void copy_values(em_class_t *class, em_value_t instance) {

	if (EM_VALUE_OK(class->clsbase))
		copy_values(EM_CLASS(EM_OBJECT_FROM_VALUE(class->clsbase)), instance);

	em_map_entry_t *entry = EM_MAP(EM_OBJECT_FROM_VALUE(class->map))->first;
	while (entry) {

		if (em_is_function(entry->value) ||
		    em_is_builtin_function(entry->value))
			em_map_set(instance, entry->key, em_method_new(instance, entry->value));
		else em_map_set(instance, entry->key, entry->value);

		entry = entry->next;
	}
}

/* instantiate class */
static em_value_t class_call(em_context_t *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_class_t *class = EM_CLASS(EM_OBJECT_FROM_VALUE(v));

	em_value_t instance = em_map_new();
	copy_values(class, instance);

	em_value_t call = em_util_get_value(class->map, "_initialize");
	if (EM_VALUE_OK(call)) {

		em_value_t newargs[EM_FUNCTION_MAX_ARGUMENTS+1] = {instance};
		memcpy(newargs+1, args, nargs * sizeof(em_value_t));

		em_value_incref(v);
		em_value_t result = em_value_call(context, call, newargs, nargs+1, pos);
		em_value_decref_no_free(v);

		if (!EM_VALUE_OK(result)) {

			em_value_delete(instance);
			return EM_VALUE_FAIL;
		}
	}

	em_util_set_value(instance, "_class", v);
	return instance;
}

/* get value by hash */
static em_value_t class_get_by_hash(em_value_t v, em_hash_t hash, em_pos_t *pos) {

	return em_value_get_by_hash(EM_CLASS(EM_OBJECT_FROM_VALUE(v))->map, hash, pos);
}

/* get value by index */
static em_value_t class_get_by_index(em_value_t v, em_value_t i, em_pos_t *pos) {

	return em_value_get_by_index(EM_CLASS(EM_OBJECT_FROM_VALUE(v))->map, i, pos);
}

/* get string representation */
static em_value_t class_to_string(em_value_t v, em_pos_t *pos) {

	em_class_t *class = EM_CLASS(EM_OBJECT_FROM_VALUE(v));

	char buf[128];
	snprintf(buf, 128, "<Class '%s'>", class->name);
	return em_string_new_from_utf8(buf, em_utf8_strlen(buf));
}

/* destroy class */
static void class_free(void *p) {

	em_class_t *class = EM_CLASS(p);

	em_value_decref(class->map);
	em_value_decref(class->clsbase);
	EM_NODE_DECREF(class->node);
}

/* create bound method */
EM_API em_value_t em_method_new(em_value_t binding, em_value_t function) {

	em_value_t value = em_object_new(&method_type, sizeof(em_method_t));
	em_method_t *method = EM_METHOD(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(method)->free = method_free;

	em_value_incref(function);

	method->binding = binding;
	method->function = function;

	return value;
}

/* create class */
EM_API em_value_t em_class_new(em_node_t *node, const char *name, em_value_t base, em_value_t map) {

	em_value_t value = em_object_new(&class_type, sizeof(em_class_t));
	em_class_t *class = EM_CLASS(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(class)->free = class_free;

	EM_NODE_INCREF(node);
	em_value_incref(base);
	em_value_incref(map);

	class->node = node;
	class->name = name;
	class->clsbase = base;
	class->map = map;

	return value;
}

/* check if value is method */
EM_API em_bool_t em_is_method(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &method_type;
}

/* check if value is class */
EM_API em_bool_t em_is_class(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &class_type;
}

/* check if a class inherits a base class */
EM_API em_bool_t em_class_inherits(em_value_t cls, em_value_t base) {

	if (!em_is_class(cls) || !em_is_class(base))
		return EM_FALSE;

	em_class_t *current = EM_CLASS(EM_OBJECT_FROM_VALUE(cls));
	while (current) {

		if (current == EM_CLASS(EM_OBJECT_FROM_VALUE(base)))
			return EM_TRUE;
		current = EM_CLASS(EM_OBJECT_FROM_VALUE(current->clsbase));
	}
	return EM_FALSE;
}

/*
 * Copyright 2026, Elliot Kohlmyer
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
#include <emerald/string.h>
#include <emerald/none.h>
#include <emerald/context.h>
#include <emerald/function.h>

/* builtin function type */
static em_value_t builtin_call(struct em_context *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos);
static em_value_t builtin_to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t builtin_type = {
	.call = builtin_call,
	.to_string = builtin_to_string,
};

/* function type */
static em_value_t call(struct em_context *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos);
static em_value_t to_string(em_value_t v, em_pos_t *pos);

static em_object_type_t type = {
	.call = call,
	.to_string = to_string,
};

/* call builtin function */
static em_value_t builtin_call(struct em_context *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_builtin_function_t *function = EM_BUILTIN_FUNCTION(EM_OBJECT_FROM_VALUE(v));
	return function->handler(context, args, nargs, pos);
}

/* get string representation of builtin function */
static em_value_t builtin_to_string(em_value_t v, em_pos_t *pos) {

	em_builtin_function_t *function = EM_BUILTIN_FUNCTION(EM_OBJECT_FROM_VALUE(v));

	char buf[128];
	snprintf(buf, 128, "<Builtin function '%s'>", function->name);
	return em_string_new_from_utf8(buf, em_utf8_strlen(buf));
}

/* call function */
static em_value_t call(struct em_context *context, em_value_t v, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_function_t *function = EM_FUNCTION(EM_OBJECT_FROM_VALUE(v));
	if (nargs != function->nargnames) {

		if (nargs > function->nargnames)
			em_log_runtime_error(pos, "Too many arguments to function '%s'", function->name);
		else em_log_runtime_error(pos, "Too few arguments to function '%s'", function->name);

		return EM_VALUE_FAIL;
	}

	if (em_context_push_scope(context) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	for (size_t i = 0; i < nargs; i++)
		em_context_set_value(context, em_utf8_strhash(function->argnames[i]), args[i]);

	em_value_t result = em_context_visit(context, function->body_node);

	/* pop_scope may or may not delete result, so prevent it from doing so */
	em_value_incref(result);
	em_context_pop_scope(context);
	em_value_decref(result);

	if (em_log_catch(&em_class_system_return)) {

		em_log_clear();
		return context->pass;
	}
	return EM_VALUE_OK(result)? em_none: EM_VALUE_FAIL;
}

/* get string representation of function */
static em_value_t to_string(em_value_t v, em_pos_t *pos) {

	em_function_t *function = EM_FUNCTION(EM_OBJECT_FROM_VALUE(v));

	char buf[128];
	snprintf(buf, 128, "<Function '%s'>", function->name);
	return em_string_new_from_utf8(buf, em_utf8_strlen(buf));
}

/* free function */
static void function_free(void *p) {

	em_function_t *function = EM_FUNCTION(p);

	EM_NODE_DECREF(function->function_node);
}

/* create builtin function */
EM_API em_value_t em_builtin_function_new(const char *name, em_builtin_function_handler_t handler) {

	em_value_t value = em_object_new(&builtin_type, sizeof(em_builtin_function_t));
	em_builtin_function_t *function = EM_BUILTIN_FUNCTION(EM_OBJECT_FROM_VALUE(value));

	function->name = name;
	function->handler = handler;

	return value;
}

/* create function */
EM_API em_value_t em_function_new(em_node_t *function_node, em_node_t *body_node, const char *name, size_t nargnames, const char **argnames) {

	em_value_t value = em_object_new(&type, sizeof(em_function_t) + nargnames * sizeof(const char *));
	em_function_t *function = EM_FUNCTION(EM_OBJECT_FROM_VALUE(value));

	EM_REFOBJ(function)->free = function_free;

	function->function_node = EM_NODE_INCREF(function_node);
	function->body_node = body_node;
	function->name = name;
	function->nargnames = nargnames;
	memcpy(function->argnames, argnames, nargnames * sizeof(const char *));

	return value;
}

/* check if value is builtin function */
EM_API em_bool_t em_is_builtin_function(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &builtin_type;
}

/* check if value is function */
EM_API em_bool_t em_is_function(em_value_t v) {

	return v.type == EM_VALUE_TYPE_OBJECT && EM_OBJECT_FROM_VALUE(v)->type == &type;
}

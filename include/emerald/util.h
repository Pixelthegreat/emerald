/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Handy module utilities
 *
 * Format guide for em_util_parse_args:
 *   - v: any value
 *   - n: number
 *   - i: integer value
 *   - f: floating point value
 *   - o: object
 *   - w: wide string object
 *   - W: wide string data pointer
 *   - l: list
 *   - m: map
 *   - b: byte array
 * The '*' character works like a regex zero or more
 * repeat, and should if necessary follow the last
 * token.
 *
 * The '~' character comes before a format character
 * and indicates that 'none' is a valid value. This
 * rule does not apply when followed by 'i' or 'f'.
 */
#ifndef EMERALD_UTIL_H
#define EMERALD_UTIL_H

#include <stdarg.h>
#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/context.h>
#include <emerald/map.h>
#include <emerald/function.h>

/* functions */
EM_API void em_util_set_value(em_value_t map, const char *name, em_value_t value); /* set value with utf8 name */
EM_API em_value_t em_util_get_value(em_value_t map, const char *name); /* get value with utf8 name */
EM_API void em_util_set_string(em_value_t map, const char *name, const char *value); /* set value with utf8 name to utf8 string */
EM_API void em_util_set_function(em_value_t map, const char *name, em_builtin_function_handler_t function); /* builtin function shorthand */
EM_API void em_util_set_class_value(em_value_t cls, const char *name, em_value_t value); /* set value in class */
EM_API void em_util_set_class_method(em_value_t cls, const char *name, em_builtin_function_handler_t function); /* set method in class */

EM_API em_result_t em_util_parse_vargs(em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, va_list vargs); /* parse arguments with variadic list */
EM_API em_result_t em_util_parse_args(em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, ...); /* parse arguments */

#endif /* EMERALD_UTIL_H */

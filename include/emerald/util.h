/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Handy module utilities
 *
 * Format guide for em_util_parse_args:
 *   - v: any value
 *   - n: number
 *   - i: integer
 *   - f: floating point
 *   - o: object
 *   - w: wide string object
 *   - W: wide string data pointer
 *   - l: list
 *   - m: map
 */
#ifndef EMERALD_UTIL_H
#define EMERALD_UTIL_H

#include <stdarg.h>
#include <emerald/core.h>
#include <emerald/value.h>
#include <emerald/context.h>
#include <emerald/function.h>

/* functions */
EM_API void em_util_set_value(em_context_t *context, const char *name, em_value_t value); /* set value with utf8 name */
EM_API void em_util_set_function(em_context_t *context, const char *name, em_builtin_function_handler_t function); /* builtin function shorthand */
EM_API em_result_t em_util_parse_vargs(em_context_t *context, em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, va_list vargs); /* parse arguments with variadic list */
EM_API em_result_t em_util_parse_args(em_context_t *context, em_pos_t *pos, em_value_t *args, size_t nargs, const char *format, ...); /* parse arguments */

#endif /* EMERALD_UTIL_H */

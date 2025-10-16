/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Logging utilities
 */
#ifndef EMERALD_LOG_H
#define EMERALD_LOG_H

#include <stdarg.h>
#include <emerald/core.h>

/* log levels */
typedef enum em_log_level {
	EM_LOG_LEVEL_INFO = 0,
	EM_LOG_LEVEL_WARNING,
	EM_LOG_LEVEL_ERROR,
	EM_LOG_LEVEL_FATAL,

	EM_LOG_LEVEL_COUNT,
} em_log_level_t;

EM_API em_log_level_t em_log_hide_level;

/* error position */
typedef struct em_pos {
	const char *path; /* file path */
	const char *text; /* file contents */
	em_ssize_t len; /* length of text */
	em_ssize_t index; /* index into file contents */
	em_ssize_t lastchsz; /* size of last character in bytes */
	em_ssize_t line, column; /* line and column numbers */
	em_ssize_t lstart, lend; /* start and end indices of line */
	int cc; /* current character */
	struct em_context *context; /* context */
} em_pos_t;

#define EM_POS_INIT ((em_pos_t){NULL, NULL, 0, -1, 1, 0, 0, -1, -1, 0, NULL})

/* builtin error classes */
EM_API struct em_value em_class_error;
EM_API struct em_value em_class_syntax_error;
EM_API struct em_value em_class_runtime_error;
EM_API struct em_value em_class_system_break;
EM_API struct em_value em_class_system_continue;
EM_API struct em_value em_class_system_return;
EM_API struct em_value em_class_system_exit;

/* functions */
EM_API void em_pos_advance(em_pos_t *pos); /* advance position */

EM_API void em_log(em_log_level_t level, const char *file, long line, const char *fmt, ...); /* log a basic message */
EM_API void em_log_error(const em_pos_t *pos, const char *fmt, ...); /* log an error */
EM_API void em_log_verror(const em_pos_t *pos, const char *fmt, va_list args); /* log an error with va_list */
EM_API void em_log_raise(struct em_value *cls, const em_pos_t *pos, const char *fmt, ...); /* raise an error */
EM_API const char *em_log_get_message(void); /* get raised error message */
EM_API em_bool_t em_log_catch(struct em_value *cls); /* check if raised error has such name */
EM_API void em_log_clear(void); /* clear raised error */
EM_API void em_log_flush(void); /* print raised error if present */
EM_API void em_log_begin(em_log_level_t level); /* begin log message */
EM_API void em_log_printf(const char *fmt, ...); /* print log message */
EM_API void em_log_vprintf(const char *fmt, va_list args); /* print log message with va_list */
EM_API void em_log_end(void); /* end log message */

EM_API void em_error_instantiate(struct em_value *value, struct em_value *cls, const char *message); /* instantiate an error */

#define em_log_syntax_error(pos, ...) em_log_raise(&em_class_syntax_error, pos, __VA_ARGS__)
#define em_log_runtime_error(pos, ...) em_log_raise(&em_class_runtime_error, pos, __VA_ARGS__)

#define em_log_info(...) em_log(EM_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define em_log_warning(...) em_log(EM_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define em_log_fatal(...) em_log(EM_LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif /* EMERALD_LOG_H */

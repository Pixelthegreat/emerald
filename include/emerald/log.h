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
} em_pos_t;

#define EM_POS_INIT ((em_pos_t){NULL, NULL, 0, -1, 1, 0, 0, -1, -1, 0})

/* functions */
EM_API void em_pos_advance(em_pos_t *pos); /* advance position */

EM_API void em_log(em_log_level_t level, const char *file, long line, const char *fmt, ...); /* log a basic message */
EM_API void em_log_error(const em_pos_t *pos, const char *fmt, ...); /* log an error */
EM_API void em_log_verror(const em_pos_t *pos, const char *fmt, va_list args); /* log an error with va_list */
EM_API void em_log_raise(const char *name, const em_pos_t *pos, const char *fmt, ...); /* raise an error */
EM_API em_bool_t em_log_catch(const char *name); /* check if raised error has such name */
EM_API void em_log_flush(void); /* print raised error if present */
EM_API void em_log_begin(em_log_level_t level); /* begin log message */
EM_API void em_log_printf(const char *fmt, ...); /* print log message */
EM_API void em_log_vprintf(const char *fmt, va_list args); /* print log message with va_list */
EM_API void em_log_end(void); /* end log message */

#define em_log_syntax_error(pos, ...) em_log_raise("SyntaxError", pos, __VA_ARGS__)

#define em_log_info(...) em_log(EM_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define em_log_warning(...) em_log(EM_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define em_log_fatal(...) em_log(EM_LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif /* EMERALD_LOG_H */

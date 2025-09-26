/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <emerald/core.h>
#include <emerald/utf8.h>
#include <emerald/log.h>

#ifdef DEBUG
em_log_level_t em_log_hide_level = EM_LOG_LEVEL_INFO;
#else
em_log_level_t em_log_hide_level = EM_LOG_LEVEL_ERROR;
#endif

#define LINEBUFSZ 128
static char linebuf[LINEBUFSZ];
static FILE *logfile = NULL;

/* raised error */
static em_bool_t err = EM_FALSE;
static em_bool_t printerr = EM_FALSE;

#define ERRNAMESZ 128
static char errname[ERRNAMESZ];

#define ERRBUFSZ 1024
static char errbuf[ERRBUFSZ];
static char *errbufp = errbuf;

/* log level names */
#define COL_GREEN "\e[32m"
#define COL_YELLOW "\e[33m"
#define COL_RED "\e[31m"
#define COL_GRAY "\e[37m"
#define COL_RESET "\e[39m"

static const char *levelnames[EM_LOG_LEVEL_COUNT] = {
	COL_GREEN "Info" COL_RESET,
	COL_YELLOW "Warning" COL_RESET,
	COL_RED "Error" COL_RESET,
	COL_RED "Fatal" COL_RESET,
};

/* advance position */
EM_API void em_pos_advance(em_pos_t *pos) {

	if (pos->lastchsz < 0) return; /* prior error */

	pos->index += pos->lastchsz;
	pos->column++;
	if (pos->index >= 0 && pos->index < pos->len) {

		pos->cc = em_utf8_getch(&pos->text[pos->index], &pos->lastchsz);
		if (pos->cc < 0) return;
	}
	else {
		
		pos->cc = 0;
		return;
	}

	/* adjust line and column values, and determine the range of the line */
	if (pos->cc == '\n' || pos->lstart < 0) {

		pos->line++;
		pos->column = (pos->lstart < 0)? 1: 0;

		pos->lstart = (pos->lstart < 0)? 0: pos->index+1;
		em_ssize_t i = pos->lstart;

		while (i < pos->len && pos->text[i] != '\n')
			i++;
		pos->lend = i;
	}
}

/* log a basic message */
EM_API void em_log(em_log_level_t level, const char *file, long line, const char *fmt, ...) {

	if (level < em_log_hide_level) return;

	em_log_begin(level);

	em_log_printf(":%s:%ld: ", file, line);

	va_list args;
	va_start(args, fmt);
	em_log_vprintf(fmt, args);
	va_end(args);

	em_log_end();
}

/* log an error */
EM_API void em_log_error(const em_pos_t *pos, const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);
	em_log_verror(pos, fmt, args);
	va_end(args);
}

/* log an error with va_list */
EM_API void em_log_verror(const em_pos_t *pos, const char *fmt, va_list args) {

	em_log_begin(EM_LOG_LEVEL_ERROR);
	if (pos) em_log_printf(" (File '%s', Line %ld, Column %ld):\n  ", pos->path, pos->line, pos->column);
	else em_log_printf(": ");

	em_log_vprintf(fmt, args);

	if (pos && pos->text && pos->lstart >= 0 && pos->lend >= 0) {

		size_t len = (size_t)EM_MIN(pos->lend-pos->lstart, LINEBUFSZ-1);
		memcpy(linebuf, pos->text+pos->lstart, len);
		linebuf[len] = 0;

		em_log_printf("\n -> %s", linebuf);
	}

	em_log_end();
}

/* raise an error */
EM_API void em_log_raise(const char *name, const em_pos_t *pos, const char *fmt, ...) {

	if (err) {

		em_log_warning("Error already raised");
		return;
	}
	strncpy(errname, name, ERRNAMESZ);

	/* print the error message to a string */
	err = EM_TRUE;
	printerr = EM_TRUE;

	va_list args;
	va_start(args, fmt);
	em_log_verror(pos, fmt, args);
	va_end(args);

	printerr = EM_FALSE;
}

/* check if raised error has such name */
EM_API em_bool_t em_log_catch(const char *name) {

	if (!err) return EM_FALSE;
	else if (!name || !strcmp(errname, name))
		return EM_TRUE;
	return EM_FALSE;
}

/* clear raised error */
EM_API void em_log_clear(void) {

	if (!err) {

		em_log_warning("Error not raised");
		return;
	}

	err = EM_FALSE;
	printerr = EM_FALSE;
	errbufp = errbuf;
	return;
}

/* print raised error if present */
EM_API void em_log_flush(void) {

	if (!err) {

		em_log_warning("Error not raised");
		return;
	}

	em_log_printf("%s", errbuf);
	err = EM_FALSE;
}

/* begin log message */
EM_API void em_log_begin(em_log_level_t level) {

	if (printerr) errbufp = errbuf;

	em_log_printf("%s", levelnames[level]);
}

/* print log message */
EM_API void em_log_printf(const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);
	em_log_vprintf(fmt, args);
	va_end(args);
}

/* print log message with va_list */
EM_API void em_log_vprintf(const char *fmt, va_list args) {

	if (!logfile) logfile = stderr;

	if (!printerr) vfprintf(logfile, fmt, args);
	else {

		int nbytes = vsnprintf(errbufp, ERRBUFSZ - (size_t)(errbufp - errbuf), fmt, args);
		if (nbytes >= 0) errbufp += nbytes;
	}
}

/* end log message */
EM_API void em_log_end(void) {

	em_log_printf("\n");
}

/*
 * Copyright 2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <emerald/core.h>
#include <emerald/none.h>
#include <emerald/util.h>
#include <emerald/wchar.h>
#include <emerald/utf8.h>
#include <emerald/path.h>
#include <emerald/context.h>
#include <emerald/string.h>
#include <emerald/module/array.h>
#include <emerald/module/os.h>

#if defined _WIN32 || _WIN64
#include <windows.h>
#elif defined _ECLAIR
#include <ec.h>
#else
#include <time.h>
#include <sys/stat.h>
#endif

/* file info */
#define FLAG_READ 0x1
#define FLAG_WRITE 0x2
#define FLAG_BINARY 0x4

#define WHENCE_START 0
#define WHENCE_CURSOR 1
#define WHENCE_END 2

#define MAX_FILES 32
struct {
	em_value_t map; /* file object */
	FILE *fp; /* file pointer */
	em_inttype_t flags; /* open flags */
} files[MAX_FILES];

#define PATHBUFSZ 4096
static char pathbuf[PATHBUFSZ];

/* os module */
static em_result_t initialize(em_context_t *context, em_value_t map);
static void destroy(em_context_t *context);

em_module_t em_module_os = {
	.initialize = initialize,
	.destroy = destroy,
};

#if defined _WIN32 || _WIN64
#define OS_NAME "windows"
#elif defined _ECLAIR
#define OS_NAME "eclair-os"
#else
#define OS_NAME "posix"
#endif

/* sleep for a given amount of time */
static em_value_t os_sleep(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t value;

	if (em_util_parse_args(pos, args, nargs, "n", &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

#if defined _WIN32 || _WIN64

#elif defined _ECLAIR

#else
	long ns = 0;
	if (value.type == EM_VALUE_TYPE_INT)
		ns = (long)value.value.te_inttype * 1000000000;
	else ns = (long)(value.value.te_floattype * 1000000000);

	struct timespec ts = {
		.tv_sec = (time_t)(ns / 1000000000),
		.tv_nsec = ns % 1000000000,
	};
	nanosleep(&ts, NULL);
#endif
	return em_none;
}

/* check if file exists */
static em_value_t os_exists(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	const em_wchar_t *path;

	if (em_util_parse_args(pos, args, nargs, "W", &path) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	em_wpath_fix(pathbuf, PATHBUFSZ, path);

#if defined _WIN32 || _WIN64

#elif defined _ECLAIR

#else
	struct stat st;
	if (stat(pathbuf, &st) < 0)
		return EM_VALUE_FALSE;
	return EM_VALUE_TRUE;
#endif
}

/* open file */
static em_value_t os_openFile(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	const em_wchar_t *path;
	em_inttype_t flags;

	if (em_util_parse_args(pos, args, nargs, "Wi", &path, &flags) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	const char *modestr = "";
	switch (flags) {
		case FLAG_READ: modestr = "rb"; break;
		case FLAG_WRITE: modestr = "wb"; break;
		case FLAG_READ | FLAG_WRITE: modestr = "rb+"; break;
		case FLAG_READ | FLAG_BINARY: modestr = "rb"; break;
		case FLAG_WRITE | FLAG_BINARY: modestr = "wb"; break;
		case FLAG_READ | FLAG_WRITE | FLAG_BINARY: modestr = "rb+"; break;
		default:
			em_log_runtime_error(pos, "Invalid mode flags");
			return EM_VALUE_FAIL;
	}

	/* find slot */
	size_t i = 0;
	for (; i < MAX_FILES && files[i].fp; i++);
	if (i >= MAX_FILES) {

		em_log_runtime_error(pos, "Too many open files");
		return EM_VALUE_FAIL;
	}

	/* open file */
	em_wpath_fix(pathbuf, PATHBUFSZ, path);

	FILE *fp = fopen(pathbuf, modestr);

	if (!fp) {

		em_log_runtime_error(pos, "Can't open '%s': %s", pathbuf, strerror(errno));
		return EM_VALUE_FAIL;
	}

	/* create file object */
	files[i].map = em_map_new();
	em_value_incref(files[i].map);
	files[i].fp = fp;
	files[i].flags = flags;

	EM_MAP(EM_OBJECT_FROM_VALUE(files[i].map))->userdata = (void *)i;
	return files[i].map;
}

/* read from file */
static em_value_t os_readFile(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t map;
	em_value_t value;
	size_t nread = 0;

	if (em_util_parse_args(pos, args, nargs, "mv", &map, &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	/* validate file */
	size_t i = (size_t)EM_MAP(EM_OBJECT_FROM_VALUE(map))->userdata;
	if (i >= MAX_FILES || !em_value_is(files[i].map, map)) {

		em_log_runtime_error(pos, "Not a file");
		return EM_VALUE_FAIL;
	}
	if (!(files[i].flags & FLAG_READ)) {

		em_log_runtime_error(pos, "File is write-only");
		return EM_VALUE_FAIL;
	}

	/* read binary data */
	if (files[i].flags & FLAG_BINARY) {

		if (!em_is_byte_array(value)) {

			em_log_runtime_error(pos, "Invalid arguments");
			return EM_VALUE_FAIL;
		}
		em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));
		if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR) {

			em_log_runtime_error(pos, "Invalid byte array mode");
			return EM_VALUE_FAIL;
		}

		nread = fread(array->data, 1, array->size, files[i].fp);
		return EM_VALUE_INT((em_inttype_t)nread);
	}

	/* read text */
	else if (!em_is_string(value)) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}

	size_t nbuf = 0;
	char buf[5];

	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(value));
	while (nread < string->length) {

		if (nbuf >= sizeof(buf)-1)
			nbuf = 0;

		int ch = fgetc(files[i].fp);
		if (ch == EOF) break;

		buf[nbuf++] = (char)ch;
		buf[nbuf] = 0;

		em_ssize_t nbytes;
		ch = em_utf8_getch(buf, &nbytes);
		if (ch < 0 || nbytes < 0)
			continue;
		nbuf = 0;
		string->data[nread++] = EM_INT2WC(ch);
	}
	return EM_VALUE_INT((em_inttype_t)nread);
}

/* write to file */
static em_value_t os_writeFile(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t map;
	em_value_t value;
	size_t nwritten = 0;

	if (em_util_parse_args(pos, args, nargs, "mv", &map, &value) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	/* validate file */
	size_t i = (size_t)EM_MAP(EM_OBJECT_FROM_VALUE(map))->userdata;
	if (i >= MAX_FILES || !em_value_is(files[i].map, map)) {

		em_log_runtime_error(pos, "Not a file");
		return EM_VALUE_FAIL;
	}
	if (!(files[i].flags & FLAG_WRITE)) {

		em_log_runtime_error(pos, "File is read-only");
		return EM_VALUE_FAIL;
	}

	/* write binary data */
	if (files[i].flags & FLAG_BINARY) {

		if (!em_is_byte_array(value)) {

			em_log_runtime_error(pos, "Invalid arguments");
			return EM_VALUE_FAIL;
		}
		em_byte_array_t *array = EM_BYTE_ARRAY(EM_OBJECT_FROM_VALUE(value));
		if (array->mode != EM_BYTE_ARRAY_MODE_UNSIGNED_CHAR) {

			em_log_runtime_error(pos, "Invalid byte array mode");
			return EM_VALUE_FAIL;
		}

		nwritten = fwrite(array->data, 1, array->size, files[i].fp);
		return EM_VALUE_INT((em_inttype_t)nwritten);
	}

	/* write text */
	else if (!em_is_string(value)) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}
	char buf[5];

	em_string_t *string = EM_STRING(EM_OBJECT_FROM_VALUE(value));
	for (; nwritten < string->length; nwritten++) {

		em_ssize_t size = 0;
		if ((size = em_utf8_putch(buf, EM_WC2INT(string->data[nwritten]))) < 0)
			continue;
		buf[size] = 0;

		for (size_t j = 0; j < strlen(buf); j++) {
			if (fputc(buf[j], files[i].fp) == EOF)
				break;
		}
	}
	return EM_VALUE_INT((em_inttype_t)nwritten);
}

/* seek in file */
static em_value_t os_seekFile(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t map;
	em_inttype_t whence;
	em_inttype_t position;

	if (em_util_parse_args(pos, args, nargs, "mii", &map, &whence, &position) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	if (whence < WHENCE_START || whence > WHENCE_END) {

		em_log_runtime_error(pos, "Invalid arguments");
		return EM_VALUE_FAIL;
	}
	int whences[3] = {SEEK_SET, SEEK_CUR, SEEK_END};
	int fwhence = whences[whence];

	/* validate file */
	size_t i = (size_t)EM_MAP(EM_OBJECT_FROM_VALUE(map))->userdata;
	if (i >= MAX_FILES || !em_value_is(files[i].map, map)) {

		em_log_runtime_error(pos, "Not a file");
		return EM_VALUE_FAIL;
	}

	if (files[i].flags & FLAG_WRITE)
		fflush(files[i].fp);
	fseek(files[i].fp, (long)position, fwhence);

	return EM_VALUE_INT((em_inttype_t)ftell(files[i].fp));
}

/* close file */
static em_value_t os_closeFile(em_context_t *context, em_value_t *args, size_t nargs, em_pos_t *pos) {

	em_value_t map;

	if (em_util_parse_args(pos, args, nargs, "m", &map) != EM_RESULT_SUCCESS)
		return EM_VALUE_FAIL;

	/* validate file */
	size_t i = (size_t)EM_MAP(EM_OBJECT_FROM_VALUE(map))->userdata;
	if (i >= MAX_FILES || !em_value_is(files[i].map, map)) {

		em_log_runtime_error(pos, "Not a file");
		return EM_VALUE_FAIL;
	}

	EM_MAP(EM_OBJECT_FROM_VALUE(files[i].map))->userdata = (void *)MAX_FILES;

	fclose(files[i].fp);
	files[i].fp = NULL;

	em_value_decref(files[i].map);
	files[i].map = EM_VALUE_FALSE;

	return em_none;
}

/* initialize module */
static em_result_t initialize(em_context_t *context, em_value_t map) {

	em_value_t mod = em_map_new();
	em_util_set_value(map, "__module_os", mod);

	/* system info */
	em_value_t sysinfo = em_map_new();
	em_util_set_string(sysinfo, "name", OS_NAME);

	em_util_set_value(mod, "info", sysinfo);

	/* flags */
	em_util_set_value(mod, "read", EM_VALUE_INT(FLAG_READ));
	em_util_set_value(mod, "write", EM_VALUE_INT(FLAG_WRITE));
	em_util_set_value(mod, "binary", EM_VALUE_INT(FLAG_BINARY));

	em_util_set_value(mod, "start", EM_VALUE_INT(WHENCE_START));
	em_util_set_value(mod, "cursor", EM_VALUE_INT(WHENCE_CURSOR));
	em_util_set_value(mod, "end", EM_VALUE_INT(WHENCE_END));

	/* functions */
	em_util_set_function(mod, "sleep", os_sleep);
	em_util_set_function(mod, "exists", os_exists);

	em_util_set_function(mod, "openFile", os_openFile);
	em_util_set_function(mod, "readFile", os_readFile);
	em_util_set_function(mod, "writeFile", os_writeFile);
	em_util_set_function(mod, "seekFile", os_seekFile);
	em_util_set_function(mod, "closeFile", os_closeFile);

	return EM_RESULT_SUCCESS;
}

/* destroy module */
static void destroy(em_context_t *context) {

	for (size_t i = 0; i < MAX_FILES; i++) {
		if (files[i].fp) {

			em_value_decref(files[i].map);
			fclose(files[i].fp);
		}
	}
}

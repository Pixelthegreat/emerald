/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <emerald/core.h>
#include <emerald/file.h>

#define ERRBUFSZ 256
static char errbuf[ERRBUFSZ];

#if defined EM_WINDOWS
#include <windows.h>
#elif defined EM_ECLAIR
#include <ec.h>
#else
#include <sys/stat.h>
#endif

/* file operations */
static em_bool_t fp_exists(const char *path);
static void *fp_open(const char *path, const char *mode);
static size_t fp_read(void *data, void *buffer, size_t count);
static size_t fp_write(void *data, const void *buffer, size_t count);
static long fp_seek(void *data, long offset, int whence);
static void fp_close(void *data);

static em_file_ops_t file_ops = {
	.exists = fp_exists,
	.open = fp_open,
	.read = fp_read,
	.write = fp_write,
	.seek = fp_seek,
	.close = fp_close,
};

/* check if libc file exists */
static em_bool_t fp_exists(const char *path) {

#if defined EM_WINDOWS
	DWORD attributes = GetFileAttributesA(path);
	if (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY))
		return EM_TRUE;
	return EM_FALSE;
#elif defined EM_ECLAIR
	ec_stat_t st;
	if (ec_stat(path, &st) < 0) return EM_FALSE;

	return (st.flags & ECS_REG)? EM_TRUE: EM_FALSE;
#else
	struct stat st;
	if (stat(path, &st) < 0) return EM_FALSE;
	
	return S_ISREG(st.st_mode)? EM_TRUE: EM_FALSE;
#endif
}

/* open libc file */
static void *fp_open(const char *path, const char *mode) {

	FILE *fp = fopen(path, mode);
	if (!fp) em_set_file_error("%s", strerror(errno));

	return fp;
}

/* read from libc file */
static size_t fp_read(void *data, void *buffer, size_t count) {

	size_t nread = fread(buffer, 1, count, (FILE *)data);
	if (ferror((FILE *)data)) em_set_file_error("%s", ferror((FILE *)data));

	return nread;
}

/* write to libc file */
static size_t fp_write(void *data, const void *buffer, size_t count) {

	size_t nwritten = fwrite(buffer, 1, count, (FILE *)data);
	if (ferror((FILE *)data)) em_set_file_error("%s", ferror((FILE *)data));

	return nwritten;
}

/* seek in libc file */
static long fp_seek(void *data, long offset, int whence) {

	int result = fseek((FILE *)data, offset, whence);
	if (result < 0) em_set_file_error("%s", strerror(errno));

	return ftell((FILE *)data);
}

/* close libc file */
static void fp_close(void *data) {

	fclose((FILE *)data);
}

/* set global file operations */
EM_API void em_set_file_ops(em_file_ops_t *ops) {

	memcpy(&file_ops, ops, sizeof(file_ops));
}

/* set file error */
EM_API void em_set_file_error(const char *fmt, ...) {

	va_list args;
	va_start(args, fmt);
	vsnprintf(errbuf, ERRBUFSZ, fmt, args);
	va_end(args);
}

/* get file error */
EM_API const char *em_get_file_error(void) {

	return *errbuf? errbuf: NULL;
}

/* check if file exists */
EM_API em_bool_t em_file_exists(const char *path) {

	return file_ops.exists(path);
}

/* open file */
EM_API void *em_file_open(const char *path, const char *mode) {

	return file_ops.open(path, mode);
}

/* read from file */
EM_API size_t em_file_read(void *data, void *buffer, size_t count) {

	return file_ops.read(data, buffer, count);
}

/* write to file */
EM_API size_t em_file_write(void *data, const void *buffer, size_t count) {

	return file_ops.write(data, buffer, count);
}

/* seek in file */
EM_API long em_file_seek(void *data, long offset, int whence) {

	return file_ops.seek(data, offset, whence);
}

/* close file */
EM_API void em_file_close(void *data) {

	file_ops.close(data);
}

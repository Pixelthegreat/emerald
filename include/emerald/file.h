/*
 * Copyright 2025-2026, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef EMERALD_FILE_H
#define EMERALD_FILE_H

#include <emerald/core.h>

/* file operations */
typedef struct em_file_ops {
	em_bool_t (*exists)(const char *path); /* check if file exists */
	void *(*open)(const char *path, const char *mode); /* open file */
	size_t (*read)(void *data, void *buffer, size_t count); /* read from file */
	size_t (*write)(void *data, const void *buffer, size_t count); /* write to file */
	long (*seek)(void *data, long offset, int whence); /* seek in file */
	void (*close)(void *data); /* close file */
} em_file_ops_t;

/* functions */
EM_API void em_set_file_ops(em_file_ops_t *ops); /* set global file operations */
EM_API void em_set_file_error(const char *fmt, ...); /* set file error */
EM_API const char *em_get_file_error(void); /* get file error */

EM_API em_bool_t em_file_exists(const char *path); /* check if file exists */

EM_API void *em_file_open(const char *path, const char *mode); /* open file */
EM_API size_t em_file_read(void *data, void *buffer, size_t count); /* read from file */
EM_API size_t em_file_write(void *data, const void *buffer, size_t count); /* write to file */
EM_API long em_file_seek(void *data, long offset, int whence); /* seek in file */
EM_API void em_file_close(void *data); /* close file */

#endif /* EMERALD_FILE_H */

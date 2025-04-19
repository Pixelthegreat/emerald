/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <emerald/core.h>
#include <emerald/memory.h>
#include <emerald/wchar.h>
#include <emerald/path.h>
#include <emerald/context.h>

/* NOTE: Always leave pathbuf1 for reuse, even if used previously */
#define PATHBUFSZ 4096
static char pathbuf1[PATHBUFSZ];
static char pathbuf2[PATHBUFSZ];

/* create context */
EM_API em_context_t *em_context_new(void) {

	em_context_t *context = em_malloc(sizeof(em_context_t));
	if (!context) return NULL;

	if (em_context_init(context) != EM_RESULT_SUCCESS) {

		em_context_free(context);
		return NULL;
	}

	return context;
}

/* initialize context */
EM_API em_result_t em_context_init(em_context_t *context) {

	if (!context) return EM_RESULT_FAILURE;
	else if (context->init) {

		em_log_fatal("Context already initialized");
		return EM_RESULT_FAILURE;
	}

	if (em_lexer_init(&context->lexer) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	/* set up initial directory stack */
	context->ndirstack = 1;
	context->dirstack[0] = ".";
#ifndef DEBUG
	context->ndirstack++;
	context->dirstack[1] = EM_STDLIB_DIR;
#endif
	context->rec_first = NULL;
	context->rec_last = NULL;

	context->init = EM_TRUE;
	return EM_RESULT_SUCCESS;
}

/* run code */
EM_API em_result_t em_context_run_text(em_context_t *context, const char *path, const char *text, em_ssize_t len) {

	if (!context || !context->init) return EM_RESULT_FAILURE;

	em_lexer_reset(&context->lexer, path, text, len);
	if (em_lexer_make_tokens(&context->lexer) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	em_token_t *cur = context->lexer.first;
	while (cur) {

		printf("%s:'%s' (Line %ld, Column %ld)\n", em_get_token_type_name(cur->type), cur->value, cur->pos.line, cur->pos.column);
		cur = cur->next;
	}
}

/* push directory to stack */
extern const char *em_context_pushdir(em_context_t *context, const char *path) {

	if (!context || !context->init) return NULL;

	/* no space */
	if (context->ndirstack >= EM_CONTEXT_MAX_DIRS) {

		em_log_fatal("Reached directory stack limit");
		return NULL;
	}

	context->dirstack[context->ndirstack++] = path;
	return path;
}

/* resolve file path */
extern const char *em_context_resolve(em_context_t *context, const char *path) {

	if (!context || !context->init) return NULL;

	/* check directories */
	for (size_t i = 0; i < context->ndirstack; i++) {

		if (em_path_join(pathbuf1, PATHBUFSZ, 2, context->dirstack[i], path) != EM_RESULT_SUCCESS)
			return NULL;
		
		if (em_path_exists(pathbuf1)) {

			if (em_path_fix(pathbuf2, PATHBUFSZ, pathbuf1) != EM_RESULT_SUCCESS)
				return NULL;
			return pathbuf2;
		}
	}
	return NULL;
}

/* pop directory from stack */
extern const char *em_context_popdir(em_context_t *context) {

	if (!context || !context->init) return NULL;

	/* already at bottom */
	if (!context->ndirstack) {

		em_log_fatal("Reached bottom of directory stack");
		return NULL;
	}

	return context->dirstack[--context->ndirstack];
}

/* run code from file */
extern em_result_t em_context_run_file(em_context_t *context, em_pos_t *pos, const char *path) {

	if (!context || !context->init) return EM_RESULT_FAILURE;

	/* get file path */
	const char *rpath = em_context_resolve(context, path);
	if (!rpath) {

		em_log_raise("IOError", pos, "No such file or directory: '%s'", path);
		return EM_RESULT_FAILURE;
	}

	/* check if file has already been run */
	em_recfile_t *recfile = context->rec_first;
	while (recfile) {
		if (!strcmp(recfile->rpath, rpath))
			return EM_RESULT_SUCCESS;
		recfile = recfile->next;
	}

	/* add directory to directory stack */
	if (em_path_dirname(pathbuf1, PATHBUFSZ, rpath) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	char *buf = NULL;
	if (pathbuf1[0]) {

		size_t len = strlen(pathbuf1);
		buf = (char *)em_malloc(len+1);
		memcpy(buf, pathbuf1, len);
		buf[len] = 0;

		if (!em_context_pushdir(context, buf)) {

			em_free(buf);
			return EM_RESULT_FAILURE;
		}
	}

	/* read file */
	FILE *fp = fopen(rpath, "rb");
	if (!fp) {

		em_log_raise("IOError", pos, "%s: '%s'", strerror(errno), path);
		return EM_RESULT_FAILURE;
	}

	fseek(fp, 0, SEEK_END);
	size_t len = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *fbuf = (char *)em_malloc(len+1);
	fread(fbuf, 1, len, fp);
	fbuf[len] = 0;
	fclose(fp);

	/* make file record */
	size_t reclen = strlen(rpath);
	recfile = em_malloc(sizeof(em_recfile_t)+reclen+1);
	memcpy(recfile->rpath, rpath, reclen);
	recfile->rpath[reclen] = 0;
	recfile->next = NULL;

	/* run code */
	em_result_t res = em_context_run_text(context, recfile->rpath, fbuf, (em_ssize_t)len);

	/* clean up */
	em_free(fbuf);
	if (buf) {

		(void)em_context_popdir(context);
		em_free(buf);
	}

	/* add file to run list */
	if (res == EM_RESULT_SUCCESS) {

		if (!context->rec_first) context->rec_first = recfile;
		if (context->rec_last) context->rec_last->next = recfile;
		context->rec_last = recfile;
	}
	else em_free(recfile);
	
	return res;
}

/* destroy context */
EM_API void em_context_destroy(em_context_t *context) {

	if (!context || !context->init) return;

	em_recfile_t *recfile = context->rec_first;
	while (recfile) {

		em_recfile_t *next = recfile->next;
		em_free(recfile);
		recfile = next;
	}

	em_lexer_destroy(&context->lexer);
	context->init = EM_FALSE;
}

/* free context */
EM_API void em_context_free(em_context_t *context) {

	if (!context) return;

	em_context_destroy(context);
	free(context);
}

/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emerald.h>
#include <shell/application.h>

#define SHBUFSZ 1024
static char shbuf[SHBUFSZ];
static em_context_t context;

#define PATHBUFSZ 512
static char pathbuf[PATHBUFSZ];
static em_wchar_t wpathbuf[PATHBUFSZ];

/* arguments */
enum {
	OPT_HELP = 0,

	OPT_NO_EXIT_FREE,
	OPT_NO_PRINT_ALLOCS,

	OPT_COUNT,

	OPT_HELP_BIT = 0x1,

	OPT_NO_EXIT_FREE_BIT = 0x10000,
	OPT_NO_PRINT_ALLOCS_BIT = 0x20000,
};
static int opt_flags = 0;
static const char *arg_filename = NULL;

/* parse arguments */
static em_result_t parse_args(int argc, const char **argv) {

	for (int i = 1; i < argc; i++) {

		const char *arg = argv[i];

		/* option */
		if (arg[0] == '-') {

			/* print help */
			if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
				opt_flags |= OPT_HELP_BIT;

			/* don't free objects after program execution */
			else if (!strcmp(arg, "--no-exit-free"))
				opt_flags |= OPT_NO_EXIT_FREE_BIT;

			/* don't print and handle unresolved allocations after program execution */
			else if (!strcmp(arg, "--no-print-allocs"))
				opt_flags |= OPT_NO_PRINT_ALLOCS_BIT;

			/* unrecognized */
			else {

				em_log_fatal("Unrecognized option '%s'", arg);
				return EM_RESULT_FAILURE;
			}
		}

		/* normal argument */
		else if (!arg_filename) arg_filename = arg;
		else {

			em_log_fatal("Unexpected excess argument '%s'", arg);
			return EM_RESULT_FAILURE;
		}
	}
	return EM_RESULT_SUCCESS;
}

/* print help information */
static void print_help(const char *progname) {

	printf("Usage: %s [filename] [options]\n\nOptions:\n"
	       "    -h|--help       Display this help message\n"
	       "\nArguments:\n"
	       "    filename        The name of the file to run\n",
	       progname);
}

/* read, eval, print loop */
static void repl(void) {

	while (!!strcmp(shbuf, "exit")) {

		printf(">>> ");
		fflush(stdout);

		memset(shbuf, 0, SHBUFSZ);
		fgets(shbuf, SHBUFSZ, stdin);

		size_t len = strlen(shbuf);
		if (len && shbuf[len-1] == '\n')
			shbuf[--len] = 0;

		/* run line */
		em_value_t res = em_context_run_text(&context, "<stdin>", shbuf, len);
		if (em_log_catch(NULL)) em_log_flush();

		if (EM_VALUE_OK(res)) em_value_log(res);
		em_value_delete(res);
	}
}

/* run application */
EM_API em_result_t shell_application_run(int argc, const char **argv) {

	if (parse_args(argc, argv) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	if (opt_flags & OPT_HELP_BIT) {

		print_help(argv[0]);
		return EM_RESULT_SUCCESS;
	}

	/* initialize emerald */
	em_init_flag_t init_flags = 0;
	if (opt_flags & OPT_NO_EXIT_FREE_BIT)
		init_flags |= EM_INIT_FLAG_NO_EXIT_FREE;
	if (opt_flags & OPT_NO_PRINT_ALLOCS_BIT)
		init_flags |= EM_INIT_FLAG_NO_PRINT_ALLOCS;

	if (em_init(init_flags) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	if (em_context_init(&context) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_module_init_all(&context) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	/* interpret file or stdin */
	if (!arg_filename) repl();
	else {
		em_value_t res = em_context_run_file(&context, NULL, arg_filename);
		if (em_log_catch(NULL)) em_log_flush();

		if (EM_VALUE_OK(res)) em_value_log(res);
	}

	return EM_RESULT_SUCCESS;
}

/* clean up resources */
EM_API void shell_application_destroy(void) {

	em_context_destroy(&context);
	em_quit();
}

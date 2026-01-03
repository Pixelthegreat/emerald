/*
 * Copyright 2026, Elliot Kohlmyer
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

static em_result_t result = EM_RESULT_SUCCESS;

/* arguments */
enum {
	OPT_HELP = 0,
	OPT_LOG_INFO,
	OPT_LOG_WARNING,
	OPT_LOG_FATAL,

	OPT_NO_EXIT_FREE,
	OPT_NO_PRINT_ALLOCS,

	OPT_COUNT,

	OPT_HELP_BIT = 0x1,
	OPT_LOG_INFO_BIT = 0x2,
	OPT_LOG_WARNING_BIT = 0x4,
	OPT_LOG_FATAL_BIT = 0x8,

	OPT_NO_EXIT_FREE_BIT = 0x10000,
	OPT_NO_PRINT_ALLOCS_BIT = 0x20000,
	OPT_PRINT_ALLOC_TRAFFIC_BIT = 0x40000,
};
static int opt_flags = 0;
static const char *arg_filename = NULL;
static int arg_index = -1;

/* parse arguments */
static em_result_t parse_args(int argc, const char **argv) {

	for (int i = 1; i < argc; i++) {

		const char *arg = argv[i];

		/* option */
		if (arg[0] == '-') {

			/* print help */
			if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
				opt_flags |= OPT_HELP_BIT;

			/* log info, warning and fatal messages */
			else if (!strcmp(arg, "-li") || !strcmp(arg, "--log-info"))
				opt_flags |= OPT_LOG_INFO_BIT;

			/* log warning and fatal messages */
			else if (!strcmp(arg, "-lw") || !strcmp(arg, "--log-warning"))
				opt_flags |= OPT_LOG_WARNING_BIT;

			/* log fatal messages */
			else if (!strcmp(arg, "-lf") || !strcmp(arg, "--log-fatal"))
				opt_flags |= OPT_LOG_FATAL_BIT;

			/* don't free objects after program execution */
			else if (!strcmp(arg, "--no-exit-free"))
				opt_flags |= OPT_NO_EXIT_FREE_BIT;

			/* don't print and handle unresolved allocations after program execution */
			else if (!strcmp(arg, "--no-print-allocs"))
				opt_flags |= OPT_NO_PRINT_ALLOCS_BIT;

			/* print allocation traffic (mallocs, reallocs and frees) */
			else if (!strcmp(arg, "--print-alloc-traffic"))
				opt_flags |= OPT_PRINT_ALLOC_TRAFFIC_BIT;

			/* unrecognized */
			else {

				em_log_fatal("Unrecognized option '%s'", arg);
				return EM_RESULT_FAILURE;
			}
		}

		/* normal argument */
		else if (!arg_filename) {
			
			arg_filename = arg;
			arg_index = i;
			break;
		}
	}
	return EM_RESULT_SUCCESS;
}

/* print help information */
static void print_help(const char *progname) {

	printf("Usage: %s [options] [filename] [script arguments]\n\nOptions:\n"
	       "    -h|--help          Display this help message\n"
	       "    -li|--log-info     Log info, warning and fatal messages\n"
	       "    -lw|--log-warning  Log warning and fatal messages\n"
	       "    -lf|--log-fatal    Log fatal messages\n"
	       "\nArguments:\n"
	       "    filename           The name of the file to run\n",
	       progname);
}

/* read, eval, print loop */
static void repl(void) {

	em_bool_t running = EM_TRUE;
	while (running) {

		printf(">>> ");
		fflush(stdout);

		memset(shbuf, 0, SHBUFSZ);
		fgets(shbuf, SHBUFSZ, stdin);

		size_t len = strlen(shbuf);
		if (len && shbuf[len-1] == '\n')
			shbuf[--len] = 0;

		/* run line */
		em_value_t res = em_context_run_text(&context, "<stdin>", shbuf, len);

		if (em_log_catch(&em_class_system_exit)) {

			result = EM_RESULT_FROM_CODE(context.pass.value.te_inttype);
			running = EM_FALSE;
		}
		else if (em_log_catch(NULL)) {

			result = EM_RESULT_FAILURE;
			em_log_flush();
		}

		/* print value */
		em_pos_t pos = {
			.path = "<stdin>",
			.text = NULL,
			.len = 0,
			.index = 0,
			.lastchsz = 0,
			.line = 1,
			.column = 1,
			.lstart = 0,
			.lend = 0,
			.cc = 0,
			.context = &context,
		};
		if (EM_VALUE_OK(res)) em_value_print(res, &pos);
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

	if (opt_flags & OPT_LOG_INFO_BIT)
		em_log_hide_level = EM_LOG_LEVEL_INFO;
	if (opt_flags & OPT_LOG_WARNING_BIT)
		em_log_hide_level = EM_LOG_LEVEL_WARNING;
	if (opt_flags & OPT_LOG_FATAL_BIT)
		em_log_hide_level = EM_LOG_LEVEL_ERROR;

	/* initialize emerald */
	em_init_flag_t init_flags = 0;
	if (opt_flags & OPT_NO_EXIT_FREE_BIT)
		init_flags |= EM_INIT_FLAG_NO_EXIT_FREE;
	if (opt_flags & OPT_NO_PRINT_ALLOCS_BIT)
		init_flags |= EM_INIT_FLAG_NO_PRINT_ALLOCS;
	if (opt_flags & OPT_PRINT_ALLOC_TRAFFIC_BIT)
		init_flags |= EM_INIT_FLAG_PRINT_ALLOC_TRAFFIC;

	if (em_init(init_flags) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	if (em_context_init(&context, arg_index >= 0? argv + arg_index: NULL) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;
	if (em_module_init_all(&context) != EM_RESULT_SUCCESS)
		return EM_RESULT_FAILURE;

	/* interpret file or stdin */
	if (!arg_filename) repl();
	else {
		em_value_t res = em_context_run_file(&context, NULL, arg_filename);

		if (em_log_catch(&em_class_system_exit))
			result = EM_RESULT_FROM_CODE(context.pass.value.te_inttype);

		else if (em_log_catch(NULL)) {

			result = EM_RESULT_FAILURE;
			em_log_flush();
		}
		em_value_delete(res);
	}

	return result;
}

/* clean up resources */
EM_API void shell_application_destroy(void) {

	em_module_destroy_all(&context);
	em_context_destroy(&context);
	em_quit();
}

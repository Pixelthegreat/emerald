/*
 * Copyright 2025, Elliot Kohlmyer
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <shell/application.h>

int main(int argc, const char **argv) {

	em_result_t res = shell_application_run(argc, argv);
	shell_application_destroy();
	return res == EM_RESULT_FAILURE? 1: EM_RESULT_TO_CODE(res);
}

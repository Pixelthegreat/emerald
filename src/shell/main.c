#include <stdio.h>
#include <stdlib.h>
#include <emerald/core.h>
#include <shell/application.h>

int main(int argc, const char **argv) {

	em_result_t res = shell_application_run(argc, argv);
	shell_application_destroy();
	return res == EM_RESULT_SUCCESS? EXIT_SUCCESS: EXIT_FAILURE;
}

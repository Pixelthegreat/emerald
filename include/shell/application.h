#ifndef SHELL_APPLICATION_H
#define SHELL_APPLICATION_H

#include <emerald/core.h>

/* functions */
EM_API em_result_t shell_application_run(int argc, const char **argv); /* run application */
EM_API void shell_application_destroy(void); /* clean up resources */

#endif /* SHELL_APPLICATION_H */

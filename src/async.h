#include <unistd.h>
#include <stdlib.h>
#include "assert.h"

#define async(fn_name, ...) pid_t fn_name(__VA_ARGS__) { \
    pid_t pid = fork(); \
    passert_gte(unsigned long, "%lu", pid, 0); \
    if (pid != 0) { \
        return pid; \
    } /* fallthrough */
#define async_end() exit(0); }

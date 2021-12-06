#include <stdlib.h>
#include <unistd.h>

/**
    Calls fork(), calling callback if executed in the child process and returning the child process ID in the parent process.
    Panics if fork() failed.
**/
pid_t lfork(void (*callback)(void)) {
    pid_t pid = fork();
    if (pid == 0) {
        callback();
        exit(0);
    } else if (pid > 0) {
        return pid;
    } else {
        perror("Couldn't fork()");
        exit(1);
    }
}

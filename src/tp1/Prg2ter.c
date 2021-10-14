#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

void exit_error(const char* msg, int code) {
    perror(msg);
    exit(code);
}

int main() {
    pid_t child_pid;
    int child_status;

    child_pid = fork();
    if (child_pid == -1) {
        exit_error("Error: Couldn't fork!", 1);
    } else if (child_pid == 0) {
        printf("Child process:  My PID is %d and my parent's PID is %d!\n", getpid(), getppid());
        exit(0);
    } else {
        printf("Parent process: My PID is %d and my child's PID is %d!\n", getpid(), child_pid);
        printf("Parent process: Waiting on child...\n");

        struct rusage usage;

        wait4(child_pid, &child_status, WUNTRACED, &usage);
        printf("Parent process: Child finished with status %d.\n", WEXITSTATUS(child_status));
        printf("Parent process: Child's CPU time used (user) = %llds + %lldμs\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
        printf("Parent process: Child's CPU time used (system) = %llds + %lldμs\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
    }
}

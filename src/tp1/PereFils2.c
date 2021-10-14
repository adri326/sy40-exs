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

        pid_t res_pid = wait3(&child_status, WUNTRACED, NULL);
        printf("Parent process: Child %d finished with status %d.\n", res_pid, WEXITSTATUS(child_status));
    }
}

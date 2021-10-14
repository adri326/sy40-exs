#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

pid_t cfork(const char* command, char* const args[]) {
    pid_t res = fork();
    if (res == -1) {
        fprintf(stderr, "Couldn't fork process: %s !\n", strerror(errno));
    } else if (res == 0) {
        if (execvp(command, args)) {
            fprintf(stderr, "Couldn't execute %s: %s !\n", command, strerror(errno));
            exit(1);
        }
        // ! -- Code path stops here (successful exec)
        exit(0); // To make weird linters happy
    } else {
        return res;
    }
}

pid_t cwait() {
    int status;
    pid_t res = wait(&status);

    if (res == -1) {
        if (errno == ECHILD) {
            fprintf(stderr, "There is no child to wait for!\n");
        } else {
            fprintf(stderr, "Error while calling wait(2): %s !\n", strerror(errno));
        }
    }

    if (!WIFEXITED(status)) {
        fprintf(stderr, "Process %d didn't exit!\n", res);
    }
    if (WEXITSTATUS(status) != 0) {
        fprintf(stderr, "Process %d returned exit code %d.\n", res, WEXITSTATUS(status));
    }

    return res;
}

int main() {
    char* const args_ls[3] = {"ls", "-l", NULL};
    pid_t pid_ls = cfork("ls", args_ls);

    char* const args_ps[3] = {"ps", "-l", NULL};
    pid_t pid_ps = cfork("ps", args_ps);

    pid_t first_exited = cwait();
    pid_t second_exited = cwait();

    if (first_exited == pid_ls) {
        printf("Command `ls -l` finished first!\n");
    } else {
        printf("Command `ps -l` finished first!\n");
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Expected 3 arguments, got %d!\n", argc);
        exit(1);
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {
        if (execlp(BUILD_DIR "/child_exec_c", "child_exec_c", argv[1], argv[2], NULL)) {
            printf("Error in executing " BUILD_DIR "/child_exec_c!\n");
            exit(1);
        }
    } else {
        int info;
        waitpid(child_pid, &info, 0);

        if (!WIFEXITED(info)) {
            fprintf(stderr, "Child didn't exit!\n");
        }

        int status = WEXITSTATUS(info);
        if (status != 0) {
            fprintf(stderr, "Child returned with exit code %d.\n", status);
        }
    }
}

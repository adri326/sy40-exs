#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define PIPE_ERR 2
#define FORK_ERR 3
#define EXEC_ERR 4

int main(int argc, char* argv[]) {
    int pfd[2];
    if (pipe(pfd) == -1) exit(PIPE_ERR);

    pid_t pid = fork();
    if (pid == -1) {
        exit(FORK_ERR);
    } else if (pid == 0) {
        close(pfd[1]);
        close(0);
        dup(pfd[0]);
        close(pfd[0]);

        if (execlp("wc", "wc", "-l", NULL)) {
            fprintf(stderr, "Couldn't execute 'wc -l'!\n");
            exit(EXEC_ERR);
        }
    } else {
        close(pfd[0]);
        close(1);
        dup(pfd[1]);
        close(pfd[1]);

        if (execlp("ls", "ls", "-l", argc > 1 ? argv[1] : NULL, NULL)) {
            fprintf(stderr, "Couldn't execute 'ls -l'!\n");
            exit(EXEC_ERR);
        }
    }
}

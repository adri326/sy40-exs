#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#ifdef PARENT

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        if (execl(BUILD_DIR "/fork_exec_child", "fork_exec_child", NULL)) {
            fprintf(stderr, "Couldn't execute " BUILD_DIR "/fork_exec_child !\n");
            return 1;
        }
    } else {
        int info;
        waitpid(pid, &info, 0);
        int status = WEXITSTATUS(info);
        if (status != 0) {
            printf("Child returned with exit code %d.\n", status);
        }

        printf("Je suis le père!\n");
    }
}

#else // CHILD

int main() {
    printf("Je suis le fils!\n");
    return 0;
}

#endif // PARENT

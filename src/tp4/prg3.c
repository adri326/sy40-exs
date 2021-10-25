#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    sigset_t set;
    printf("PARENT: Begin\n");

    if (argc != 2) {
        fprintf(stderr, "Expected 1 argument, got %d! Try running '%s 1'\n", argc - 1, argv[0]);
        exit(1);
    }

    int seconds = atoi(argv[1]);

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    printf("PARENT: Install mask\n");
    // sigprocmask(SIG_SETMASK, &set, NULL); // α

    if (fork() == 0) {
        // sigprocmask(SIG_SETMASK, &set, NULL); // β
        printf("CHILD:  Begin\n");
        sleep(seconds);
        printf("CHILD:  End\n");
    } else {
        sigprocmask(SIG_SETMASK, &set, NULL); // γ
        sleep(seconds);
        printf("PARENT: End\n");
    }

    return 0;
}

// if α is commented, β is uncommented and γ is commented, then the child process continues when ^C is sent even though the parent stops
// if α and β are commented but γ is uncommented, then when pressing ^C the child never ends

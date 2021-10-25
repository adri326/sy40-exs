#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#define TO_SEND 100

int sent = 0, received = 0;

void handle_sigusr1(int num) {
    assert(num == SIGUSR1);
    received++;
    return;
}

int main(int argc, char* argv[]) {
    signal(SIGUSR1, handle_sigusr1);

    pid_t pid = fork();
    if (pid == 0) {
        for (sent = 0; sent < TO_SEND; sent++) {
            assert(kill(getppid(), SIGUSR1) == 0);
        }
        printf("CHILD:  %d signals sent.\n", sent);
    } else {
        while (wait(NULL) == -1);
        printf("PARENT: %d signals received.\n", received);
    }
}

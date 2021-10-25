#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#define TO_SEND 100
#define N_SIG SIGRTMIN

int sent = 0, received = 0;
pid_t child_pid;

void handle_sigusr1_child(int num) {
    assert(num == SIGUSR1);
    // sent++;
    printf("CHILD:  Parent received signal %d\n", sent);

    sigset_t pending;
    sigpending(&pending);
    for (size_t i = 1; i < SIGRTMIN; i++) {
        if (sigismember(&pending, i)) {
            printf("Signal %d is waiting (%s)!\n", i, strsignal(i));
        }
    }
}

void handle_sigusr1_parent(int num) {
    assert(num == SIGUSR1);
    received++;
    printf("Parent: Received %d-th signal\n", received);
    kill(child_pid, SIGUSR1); // Notify child that we received its signal
}

int main(int argc, char* argv[]) {
    sigset_t current, pending;
    // signal(SIGUSR1, handle_sigusr1);

    // Block all (blockable) signals
    sigfillset(&current);
    assert(sigprocmask(SIG_SETMASK, &current, NULL) >= 0);

    // Check that no signal is pending
    sigpending(&pending);
    for (size_t i = 1; i < SIGRTMIN; i++) {
        if (sigismember(&pending, i)) {
            printf("Signal %d is waiting (%s)!\n", i, strsignal(i));
        }
    }

    // Repurpose current
    sigemptyset(&current);

    pid_t pid = fork();
    if (pid == 0) {
        // We keep the interrupt mask as-is

        signal(SIGUSR1, handle_sigusr1_child);
        for (sent = 0; sent < TO_SEND; sent++) {
            printf("CHILD:  Sending signal %d\n", sent);
            assert(kill(getppid(), SIGUSR1) == 0);
            printf("CHILD:  Sent signal %d\n", sent);
            sigsuspend(&current); // Temporarily remove the block on sigusr1
        }
        printf("CHILD:  %d signals sent.\n", sent);
    } else {
        signal(SIGUSR1, handle_sigusr1_parent);
        child_pid = pid;
        sigprocmask(SIG_SETMASK, &current, NULL);
        while (wait(NULL) == -1);
        printf("PARENT: %d signals received.\n", received);
    }
}

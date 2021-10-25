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

bool child_received_signal = false;
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

    child_received_signal = true;
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
        signal(SIGUSR1, handle_sigusr1_child);
        // sigaddset(&current, SIGUSR1);
        sigprocmask(SIG_SETMASK, &current, NULL);
        for (sent = 0; sent < TO_SEND; sent++) {
            printf("CHILD:  Sending signal %d\n", sent);
            child_received_signal = false;
            assert(kill(getppid(), SIGUSR1) == 0);
            // for (size_t i = 0; i < 100000; i++); // Uncomment to highlight the problem: handle_sigusr1_child can be called before pause
            printf("CHILD:  Sent signal %d\n", sent);
            if (!child_received_signal) pause();
            printf("CHILD:  Pause finish\n", sent);
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

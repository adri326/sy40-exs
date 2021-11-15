#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>

/**
    Calls fork(), calling callback if executed in the child process and returning the child process ID in the parent process.
    Panics if fork() failed.
**/
pid_t lfork(void (*callback)(void)) {
    pid_t pid = fork();
    if (pid == 0) {
        callback();
        exit(0);
    } else if (pid > 0) {
        return pid;
    } else {
        perror("Couldn't fork()");
        exit(1);
    }
}

static bool signals_received[34]; // Initialized to 0

/**
    Simple signal handler that sets the signals_received flag of `sig` to true.
    The signal is ignored after that.
    Panics if `sig` is outside of `[0; 34[`.
**/
void handle_signal_setflag(int sig) {
    assert(sig >= 0 && sig < 34);
    signals_received[sig] = true;
}

/**
    Sets signals to be ignored and their respective index in `signals_received` to be set to true.
    Panics if `sigaction` fails.
**/
void ignore_signals() {
    struct sigaction options;
    sigfillset(&options.sa_mask);
    options.sa_handler = handle_signal_setflag;
    options.sa_flags = 0;

    // Modify to your will:
    assert(sigaction(SIGUSR1, &options, NULL) == 0);
    assert(sigaction(SIGUSR2, &options, NULL) == 0);
}

// === Examples ===

void child() {
    sleep(2);
    printf("Sending signal!\n");
    kill(getppid(), SIGUSR1);
    sleep(2);
    printf("Sending signal!\n");
    kill(getppid(), SIGUSR2);
}

int main() {
    pid_t child_pid = lfork(child);
    ignore_signals();

    while (true) {
        sleep(1);
        if (signals_received[SIGUSR1]) {
            printf("SIGUSR1 received!\n");
            signals_received[SIGUSR1] = false;
        } else if (signals_received[SIGUSR2]) {
            printf("SIGUSR2 received!\n");
            signals_received[SIGUSR2] = false;
            break;
        }
    }

    waitpid(child_pid, NULL, 0);
}

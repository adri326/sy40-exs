#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

void empty_handler(int) {}

void my_sleep(unsigned int seconds) {
    if (seconds == 0) return;

    sigset_t previous, current, alarm_set;
    struct sigaction alarm_handler, old_alarm_handler;
    // Prepare sets
    sigfillset(&current);
    sigdelset(&current, SIGALRM);
    sigfillset(&alarm_set);
    sigdelset(&alarm_set, SIGALRM);

    // Prepare SA structure
    alarm_handler.sa_handler = empty_handler;
    alarm_handler.sa_flags = 0;
    sigfillset(&alarm_handler.sa_mask);

    // Block all signals
    sigprocmask(SIG_SETMASK, &current, &previous);

    sigaction(SIGALRM, &alarm_handler, &old_alarm_handler);
    // Schedule a SIGALRM signal
    alarm(seconds);
    // Listen to SIGALRM
    sigsuspend(&alarm_set);
    // Restore all signals
    sigprocmask(SIG_SETMASK, &previous, NULL);
    sigaction(SIGALRM, &old_alarm_handler, NULL);
}

int main() {
    printf("I feel sleepy...\n");
    my_sleep(1);
    printf("That's better!\n");
}

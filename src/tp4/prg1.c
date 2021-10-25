#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

void handle_sigint(int sig_num) {
    if (sig_num != SIGINT) {
        fprintf(stderr, "handle_sigint(sig_num) did not receive SIGINT: expected %d, got %d\n", SIGINT, sig_num);
    }
    printf("\n\x1b[0m\x1b[1m^C is forbidden but I'll give you a break :)\x1b[0m\n");
    exit(0);
}

void handle_sigstp(int sig_num, siginfo_t* info, void* context) {
    if (sig_num != SIGTSTP) {
        fprintf(stderr, "handle_sigstp(sig_num) did not receive SIGTSTP: expected %d, got %d\n", SIGTSTP, sig_num);
    }
    printf("\n\x1b[0m\x1b[1mSleeping more!\x1b[0m\n");
    printf("info = siginfo_t {\n");
    printf("  si_signo = %d\n", info->si_signo);
    printf("  si_code = %d\n", info->si_code);
    printf("  si_pid = %lu\n", info->si_pid);
    printf("  si_uid = %lu\n", info->si_uid);
    printf("  si_addr = %p\n", info->si_addr);
    printf("  si_status = %d\n", info->si_status);
    printf("}\n");
    // sigset_t mask;
    // sigemptyset(&mask);
    // sigaddset(&mask, SIGCONT);
    // sigsuspend(&mask);
    kill(getpid(), SIGSTOP);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: expected one argument, try '%s 1'\n", argv[0]);
        return 1;
    }

    int seconds = atoi(argv[1]);

    signal(SIGINT, handle_sigint);
    struct sigaction sigstp_handler;
    sigstp_handler.sa_sigaction = handle_sigstp;
    sigemptyset(&sigstp_handler.sa_mask);
    sigstp_handler.sa_flags = SA_SIGINFO;

    if (sigaction(SIGTSTP, &sigstp_handler, NULL) == -1) {
        perror("Couldn't register SIGTSTP handler");
        exit(1);
    }

    for (int i = 0; i < seconds; i++) {
        if (i > 0) printf("\x1b[1F\x1b[0K");
        printf("\x1b[%dm", 90 + (i % 8));
        printf("Now you wait %d seconds :)\n", seconds - i);
        sleep(1);
    }
    printf("\x1b[1F\x1b[0K\x1b[0m\x1b[1mWe hope you enjoyed your break :)\x1b[0m\n");
}

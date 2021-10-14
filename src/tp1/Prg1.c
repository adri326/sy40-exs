#include <stdio.h>
#include <unistd.h>

int main() {
    int c = 5;
    pid_t pid_fils = fork();
    c += 5;
    if (pid_fils == 0) {
        c += 5;
    } else {
        pid_t pid2 = fork();
        if (pid2 != 0) {
            c += 10;
        } else {
            c += 5;
        }
    }
    printf("PID %d: c=%d\n", getpid(), c);
}

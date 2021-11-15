#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <assert.h>
#include <wait.h>

struct message {
    long type;
    pid_t pid;
};
typedef struct message message_t;

#define passert_eq(type, print, left, right) { int line = __LINE__; \
    errno = 0; \
    type l = left; \
    type r = right; \
    if (l != r) { \
        fprintf(stderr, __FILE__ ":%d Assertion failed: " print " != " print "\n", line, l, r); \
        fprintf(stderr, __FILE__ ":%d Expected " #left " == " #right "\n", line); \
        if (errno != 0) { \
            fprintf(stderr, __FILE__ ":%d ", line); \
            perror("Note: errno is non-zero"); \
        } \
        exit(1); \
    } \
}

#if !defined(P2) && !defined(P3)
// == P1 ==
int main() {
    key_t key = ftok(BUILD_DIR "/tp5/prg1.c", '1');
    printf("[P1] Start\n");
    int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    if (msgid == -1) {
        perror("[P1] Couldn't call msgget()");
        exit(1);
    }

    printf("[P1] Called msgget() with key %ld, creating queue %d\n", key, msgid);

    pid_t pid_p2 = fork();
    assert(pid_p2 != -1);
    if (pid_p2 == 0) {
        char* arg[3];
        arg[0] = "tp5_p2";
        arg[1] = NULL;
        execv("tp5_p2", arg);
        perror("Couldn't call P2");
        exit(2);
    }

    printf("[P1] P2 started with PID %ld\n", pid_p2);

    pid_t pid_p3 = fork();
    assert(pid_p3 != -1);
    if (pid_p3 == 0) {
        char* arg[3];
        arg[0] = "tp5_p3";
        arg[1] = NULL;
        execv("tp5_p3", arg);
        perror("Couldn't call P3");
        exit(3);
    }

    printf("[P1] P3 started with PID %ld\n", pid_p3);

    for (int n = 0; n < 2; n++) {
        pid_t pid = wait(NULL);
        if (pid == pid_p2) {
            printf("[P1] P2 stopped\n");
        } else {
            printf("[P1] P3 stopped\n");
        }
    }

    printf("[P1] Closing message queue %d\n", msgid);
    assert(msgctl(msgid, IPC_RMID, NULL) == 0);

    printf("[P1] End\n");
    return 0;
}
#else
#ifdef P2

// == P2 ==
int main(int argc, char* argv) {
    size_t msg_len = sizeof(message_t) - sizeof(long);

    key_t key = ftok(BUILD_DIR "/tp5/prg1.c", '1');
    int msgid = msgget(key, 0);
    if (msgid == -1) {
        perror("[P2] Couldn't call msgget()");
        exit(1);
    }

    message_t req, res;

    req.type = 2;
    req.pid = getpid();

    passert_eq(int, "%d", msgsnd(msgid, &req, msg_len, 0), 0);

    passert_eq(int, "%d", msgrcv(msgid, &res, msg_len, 3, 0), msg_len);

    printf("[P2] My PID is %ld, P3's PID is %ld\n", getpid(), res.pid);
    return 0;
}

#else

// == P3 ==
int main(int argc, char* argv) {
    size_t msg_len = sizeof(message_t) - sizeof(long);

    key_t key = ftok(BUILD_DIR "/tp5/prg1.c", '1');
    int msgid = msgget(key, 0);
    if (msgid == -1) {
        perror("[P3] Couldn't call msgget()");
        exit(1);
    }
    printf("[P3] Called msgget(), using queue %d\n", msgid);

    message_t req, res;

    req.type = 3;
    req.pid = getpid();

    passert_eq(int, "%d", msgsnd(msgid, &req, msg_len, 0), 0);

    passert_eq(int, "%d", msgrcv(msgid, &res, msg_len, 2, 0), msg_len);

    printf("[P3] My PID is %ld, P3's PID is %ld\n", getpid(), res.pid);
    return 0;
}

#endif // P2
#endif // P2 | P3

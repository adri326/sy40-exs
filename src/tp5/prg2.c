#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <wait.h>

#define MAX_REQUESTS

#define passert_gt(type, print, left, right) { int line = __LINE__; \
    errno = 0; \
    type l = left; \
    type r = right; \
    if (l <= r) { \
        fprintf(stderr, __FILE__ ":%d Assertion failed: " print " <= " print "\n", line, l, r); \
        fprintf(stderr, __FILE__ ":%d Expected " #left " > " #right "\n", line); \
        if (errno != 0) { \
            fprintf(stderr, __FILE__ ":%d ", line); \
            perror("Note: errno is non-zero"); \
        } \
        exit(1); \
    } \
}

#define def_msgtype(name, body) struct name { \
    long type; \
    body \
}; \
typedef struct name name; \
static const size_t name##_len = sizeof(struct name) - sizeof(long);

def_msgtype(request,
    pid_t emitter;
    int n_requests;
    int n_max;
);

def_msgtype(response,
    int list[MAX_REQUESTS];
);

int main(int argc, char* argv[]) {
    srand(getpid());

    if (argc != 2) {
        fprintf(stderr, "Expected 1 integer argument!\n");
        exit(1);
    }

    int n_clients = strtol(argv[1], NULL, 10);
    passert_gt(int, "%d", n_clients, 0);

    key_t key = ftok(argv[0], 'c');
    passert_gt(key_t, "%ld", key, -1);

    int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    passert_gt(int, "%d", msgid, -1);

    assert(msgctl(msgid, IPC_RMID, NULL) == 0);
}

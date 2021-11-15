#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <wait.h>
#include "../assert.h"

#define MAX_REQUESTS 128


#define def_msgtype(name, type_id, body) struct name { \
    long type; \
    body \
}; \
typedef struct name name; \
static const size_t name##_len = sizeof(struct name) - sizeof(long); \
static const size_t name##_type = type_id;

def_msgtype(request, 1,
    pid_t emitter;
    int n_requests;
    int n_max;
);

def_msgtype(response, 2,
    int list[MAX_REQUESTS];
);


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

void client() {
    key_t key = ftok(__FILE__, 'd');
    passert_neq(key_t, "%ld", key, -1);

    int msgid = msgget(key, 0);
    passert_neq(int, "%d", msgid, -1);

    request req;
    response res;

    req.type = request_type;
    req.emitter = getpid();
    req.n_requests = rand() % MAX_REQUESTS;
    req.n_max = rand() + 1;

    passert_eq(int, "%d", msgsnd(msgid, &req, request_len, 0), 0);

    passert_eq(int, "%d", msgrcv(msgid, &res, response_len, getpid(), 0), response_len);

    char buffer[1024] = {0};
    for (size_t i = 0; i < req.n_requests; i++) {
        snprintf(buffer, 1024, "res.list[%zu] = %d\n", i, res.list[i]);
    }
    write(STDOUT_FILENO, buffer, strlen(buffer) + 1);
}

void server() {
    // TODO: receive messages and response to them
}

int main(int argc, char* argv[]) {
    srand(getpid());

    if (argc != 2) {
        fprintf(stderr, "Expected 1 integer argument!\n");
        exit(1);
    }

    size_t n_clients = strtol(argv[1], NULL, 10);
    passert_gt(size_t, "%zu", n_clients, 0);

    key_t key = ftok(__FILE__, 'd');
    passert_neq(key_t, "%ld", key, -1);

    int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    passert_neq(int, "%d", msgid, -1);

    for (size_t i = 0; i < n_clients; i++) {
        lfork(client);
    }

    server();

    for (size_t i = 0; i < n_clients; i++) {
        pid_t pid = wait(NULL);
    }

    assert(msgctl(msgid, IPC_RMID, NULL) == 0);
}

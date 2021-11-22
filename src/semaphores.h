#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "assert.h"

union semctl {
    int value;
    struct semid_ds* buffer;
    unsigned short* array;
};
typedef union semctl semctl_t;

int init_semaphores(key_t key, int n_sems, unsigned short* default_values) {
    const int FLAGS = IPC_CREAT | SEMPERM;
    int res;
    passert_gte(int, "%d", res = semget(key, n_sems, FLAGS), 0);

    semctl_t arg;
    arg.array = default_values;
    passert_gte(int, "%d", semctl(res, 0, SETALL, arg), 0);

    return res;
}

int free_semaphores(int semid) {
    passert_eq(int, "%d", semctl(semid, 0, IPC_RMID), 0);
}

int g_semaphores = 0;

void P(int sem_index) {
    struct sembuf operation = {
        .sem_num = sem_index,
        .sem_op = -1,
        .sem_flg = 0
    };

    semop(g_semaphores, &operation, 1);
}

void V(int sem_index) {
    struct sembuf operation = {
        .sem_num = sem_index,
        .sem_op = 1,
        .sem_flg = 0
    };

    semop(g_semaphores, &operation, 1);
}

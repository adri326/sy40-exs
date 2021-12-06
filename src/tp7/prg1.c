#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "../lfork.h"
#include "../assert.h"
#include "../semaphores.h"

int* shared_ptr = NULL;

void child() {
    for (size_t i = 0; i < 10000; i++) {
        P(0);
        for (size_t n = 0; n < 100; n++) {
            (*shared_ptr)++;
        }
        V(0);
    }
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(int), 0666);
    {
        unsigned short sem_init[1] = {1};
        g_semaphores = init_semaphores(IPC_PRIVATE, 1, sem_init);
    }
    shared_ptr = (int*)shmat(shmid, NULL, 0);
    passert_neq(void*, "%p", shared_ptr, (void*)-1);

    lfork(child);

    for (size_t i = 0; i < 10000; i++) {
        P(0);
        for (size_t n = 0; n < 100; n++) {
            (*shared_ptr)++;
        }
        V(0);
    }

    wait(NULL);
    passert_eq(int, "%d", *shared_ptr, 2000000);
    shmctl(shmid, IPC_RMID, NULL);
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <time.h>
#include "../lfork.h"
#include "../assert.h"
#include "../semaphores.h"

#define MAX_NUMBERS 256
void* shared_ptr = NULL;

void reader() {
    P(0);
    size_t length = *(size_t*)shared_ptr;
    passert_gte(size_t, "%zu", length, 0);

    int* data = (int*)(shared_ptr + sizeof(size_t));
    float sum = 0.0;
    for (size_t n = 0; n < length; n++) {
        sum += (float)(data[n]);
    }
    float* average = (float*)(shared_ptr + sizeof(size_t) + sizeof(int) * MAX_NUMBERS);
    *average = sum / (float)length;
    V(1);
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(size_t) + sizeof(int) * MAX_NUMBERS + sizeof(float), 0666);
    {
        unsigned short sem_init[2] = {0, 0};
        g_semaphores = init_semaphores(IPC_PRIVATE, 2, sem_init);
    }
    shared_ptr = shmat(shmid, NULL, 0);
    passert_neq(void*, "%p", shared_ptr, (void*)-1);

    lfork(reader);

    srand(time(0));
    size_t length = rand() % MAX_NUMBERS;
    *(size_t*)shared_ptr = length;
    int* data = (int*)(shared_ptr + sizeof(size_t));
    for (size_t n = 0; n < length; n++) {
        data[n] = rand() % 1000;
        printf("data[%zu] = %d\n", n, data[n]);
    }

    V(0);
    P(1);
    float average = *(float*)(shared_ptr + sizeof(size_t) + sizeof(int) * MAX_NUMBERS);
    printf("Average is %.6f\n", average);

    wait(NULL);
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include "../assert.h"
#include "../async.h"
#define SEMPERM 0600
#include "../semaphores.h"

#define N_SEMAPHORES 2
#define MAX_AGENTS 3

#define message(name, id, ...) { \
    printf("["); \
    int _i = 0; \
    for (_i = 0; _i < id; _i++) printf("   "); \
    printf("(*)"); \
    for (_i++; _i < MAX_AGENTS; _i++) printf("   "); \
    printf("] % 4s(%d): ", name, id); \
    printf(__VA_ARGS__); \
}

async(TGV, int i) {
    message("TGV", i, "Départ Paris\n");
    sleep(3);
    message("TGV", i, "Arrivée Strasbourg\n");
    V(0);
    sleep(4);
    message("TGV", i, "Départ Strasbourg\n");
    sleep(10);
    message("TGV", i, "Arrivée Basel\n");
    sleep(2);
    message("TGV", i, "Arrêt\n");
}
async_end()

async(TER, int i) {
    message("TER", i, "Attente TGV\n");
    sleep(2);
    P(0);
    message("TER", i, "Départ Strasbourg\n");
    sleep(3);
    message("TER", i, "Arrivée Mulhouse\n");
    V(1);
    sleep(5);
    message("TER", i, "Arrêt\n");
}
async_end()

async(TAXI, int i) {
    message("Taxi", i, "Attente TER\n");
    sleep(3);
    P(1);
    message("Taxi", i, "Départ Mulhouse\n");
    sleep(4);
    message("Taxi", i, "Arrivée Belfort\n");
    sleep(2);
    message("Taxi", i, "Arrêt\n");
}
async_end()

int main() {
    unsigned short sem_init[N_SEMAPHORES] = {0, 0};
    g_semaphores = init_semaphores((key_t)IPC_PRIVATE, N_SEMAPHORES, sem_init);
    pid_t pid_tgv = TGV(0);
    pid_t pid_ter = TER(1);
    pid_t pid_taxi = TAXI(2);

    int status;
    waitpid(pid_tgv, &status, 0);
    waitpid(pid_taxi, &status, 0);
    waitpid(pid_ter, &status, 0);

    free_semaphores(g_semaphores);
}

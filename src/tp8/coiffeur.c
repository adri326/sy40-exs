#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "../assert.h"

#define N_THREADS 10
#define MAX_SEATS 8

pthread_t threads[N_THREADS];
pthread_mutex_t mutex;
pthread_cond_t waiting_line, asleep;

int clients_waiting = 0;
bool barber_can_stop = false;

void coiffeur() {
    while (true) {
        pthread_mutex_lock(&mutex);
        if (clients_waiting == 0) {
            printf("The barber starts sleeping...\n");
            pthread_cond_wait(&asleep, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        printf("The barber signals the next client to come!\n");
        pthread_cond_signal(&waiting_line);
        printf("The barber works!\n");
        sleep(1);
        printf("The barber is done working.\n");
    }
}

void client(int i) {
    pthread_mutex_lock(&mutex);
    printf("Client %d enters the room!\n", i);
    if (clients_waiting < MAX_SEATS) {
        printf("Client %d sees that there are %d/%d seats taken!\n", i, clients_waiting, MAX_SEATS);
        clients_waiting++;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&asleep);

        printf("Client %d waits in line!\n", i);
        pthread_cond_wait(&waiting_line, &mutex);
        printf("Client %d is last in line!\n", i);
        clients_waiting--;
        pthread_mutex_unlock(&mutex);
        sleep(1);
        printf("Client %d was brushed and leaves.\n", i);
    } else {
        printf("Client %d sees that all seats are taken and leaves.\n", i);
    }

    pthread_mutex_unlock(&mutex);
}

void* coiffeur_entry(void* _) {
    coiffeur();

    pthread_exit(NULL);
}

void* client_entry(void* i) {
    client((int)(size_t)i);

    pthread_exit(NULL);
}

int main() {
    passert_gte(size_t, "%zu", sizeof(void*), sizeof(int));

    passert_eq(int, "%d", pthread_cond_init(&waiting_line, NULL), 0);
    passert_eq(int, "%d", pthread_cond_init(&asleep, NULL), 0);
    passert_eq(int, "%d", pthread_mutex_init(&mutex, NULL), 0);

    for (size_t i = 0; i < N_THREADS; i++) {
        pthread_attr_t attributes;
        pthread_attr_init(&attributes);
        if (i == 0) {
            passert_eq(
                int, "%d",
                pthread_create(&threads[i], &attributes, coiffeur_entry, NULL),
                0
            );
        } else {
            passert_eq(
                int, "%d",
                pthread_create(&threads[i], &attributes, client_entry, (void*)(size_t)i),
                0
            );
        }
    }

    for (size_t i = 1; i < N_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // passert_eq(int, "%d", pthread_cond_destroy(&waiting_line), 0);
    // passert_eq(int, "%d", pthread_cond_destroy(&asleep), 0);
    // passert_eq(int, "%d", pthread_mutex_destroy(&mutex), 0);

    exit(0);
}

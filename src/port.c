#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/wait.h>

/**
E1: Boat <--(SIGUSR1)--- Embark ~ suspend
E1': Boat <--(pipe)--- Embark ~ send the pid of the embark
E2: Embark ---(pipe+SIGUSR1)--> Entrepot
E3: Embark <--(pipe)--- Entrepot (when ready) ~ may block for a while
E4: Boat <--(pipe)--- Embark
E5: Boat ---(SIGUSR2)--> Embark ~ suspend

D1: Boat <--(SIGUSR2)--- Disembark ~ suspend
D2: Boat ---(pipe)--> Disembark
D3: Disembark ---(pipe+SIGUSR2)--> Entrepot
D4: Disembark <--(SIGUSR1)--- Entrepot ~ suspend
**/

/**
    Writes to the process mask to ignore SIGUSR signals
**/
void set_sigusr_procmask() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_SETMASK, &mask, NULL);
}

int wait_sigusr() {
    sigset_t mask;
    siginfo_t info;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    int res = sigwaitinfo(&mask, &info);
    return info.si_signo;
}

void docker_embark(int storage_query, int storage_result, int boat_load, pid_t boats[], size_t boat_length) {
    set_sigusr_procmask();

    for (size_t i = 0; i < boat_length; i += 2) {
        printf("[Embark]\t Handling boat %zu: PID=%ld\n", i, boats[i]);
        pid_t boat = boats[i];
        // E1: Notify boat that it is ready
        assert(kill(boat, SIGUSR1) == 0);
        // E1': Send the boat our PID
        char str_buffer[32] = {0};
        sprintf(str_buffer, "%ld", getpid());
        assert(write(boat_load, str_buffer, 32) == 32);

        // E2: Ask storage for a random number of packages
        int packages = (rand() % 3) + 1;
        sprintf(str_buffer, "%d", packages);

        assert(write(storage_query, str_buffer, 4) == 4);
        assert(kill(getppid(), SIGUSR1) == 0);

        // E3: Get the number of packages from storage
        assert(read(storage_result, str_buffer, 4) == 4);
        assert(packages == atoi(str_buffer)); // Verify that we got the right amount of packages back

        // E4: Give the packages to the boat
        assert(write(boat_load, str_buffer, 4) == 4);

        // E5: Wait for the boat's response before looping
        assert(wait_sigusr() == SIGUSR2);
    }

    exit(0);
}

void docker_disembark(int storage_put, int boat_unload, pid_t boats[], size_t boat_length) {
    sigset_t suspend_mask;
    sigfillset(&suspend_mask);
    sigdelset(&suspend_mask, SIGUSR1);
    sigdelset(&suspend_mask, SIGUSR2);
    set_sigusr_procmask();

    for (size_t i = 1; i < boat_length; i += 2) {
        printf("[Disembark]\t Handling boat %zu: PID=%ld\n", i, boats[i]);
        pid_t boat = boats[i];
        // D1: Notify boat that it is ready
        assert(kill(boat, SIGUSR2) == 0);

        // D2: Read boat's response
        char str_buffer[4] = {0};
        assert(read(boat_unload, str_buffer, 4) == 4);
        int packages = atoi(str_buffer);
        assert(packages > 0);
        printf("[Disembark]\t Boat is sending %d packages!\n", packages);

        // D3: Send packages to entrepot
        assert(write(storage_put, str_buffer, 4) == 4);
        assert(kill(getppid(), SIGUSR2) == 0);

        // D4: Wait for entrepot's answer
        assert(wait_sigusr() == SIGUSR1);
        printf("[Disembark]\t Got answer!\n");
    }
}

void boat(int embark_pipe, int disembark_pipe) {
    sigset_t suspend_mask;
    sigfillset(&suspend_mask);
    sigdelset(&suspend_mask, SIGUSR1);
    sigdelset(&suspend_mask, SIGUSR2);
    set_sigusr_procmask();

    int sig = wait_sigusr();
    assert(sig == SIGUSR1 || sig == SIGUSR2);

    if (sig == SIGUSR1) {
        // Boat is ready to be filled
        char str_buffer[32] = {0};

        // Read the embark process pid
        assert(read(embark_pipe, str_buffer, 32) == 32);
        pid_t embark = strtol(str_buffer, NULL, 10);
        // printf("Embark PID is %ld\n", embark);

        // Read the number of packages
        assert(read(embark_pipe, str_buffer, 4) == 4);
        int packages = atoi(str_buffer);
        assert(packages > 0);
        printf("[Boat %d]\t Embarked with %d packages!\n", getpid(), packages);

        // Notify embark process that we are done
        assert(kill(embark, SIGUSR2) == 0);
    } else if (sig == SIGUSR2) {
        // Boat is ready to be emptied
        int packages = (rand() % 3) + 1;
        char str_buffer[4] = {0};
        sprintf(str_buffer, "%d", packages);
        printf("[Boat %d]\t Sending %d packages!\n", getpid(), packages);

        // Send the number of packages
        assert(write(disembark_pipe, str_buffer, 4));
    }
}

void storage(
    int fd_embark_request,
    int fd_embark_response,
    int fd_disembark_request,
    pid_t embark_pid,
    pid_t disembark_pid
) {
    sigset_t suspend_mask;
    siginfo_t suspend_info;

    sigemptyset(&suspend_mask);
    sigaddset(&suspend_mask, SIGUSR1);
    sigaddset(&suspend_mask, SIGUSR2);
    sigaddset(&suspend_mask, SIGCHLD);

    struct timespec suspend_timeout;
    suspend_timeout.tv_sec = 0;
    suspend_timeout.tv_nsec = 500000;

    set_sigusr_procmask();
    int stored_packages = (rand() % 19) + 1;

    bool embark_finished = false, disembark_finished = false;
    int embark_queue = 0, disembark_queue = 0;

    while (!embark_finished || !disembark_finished) {
        int sig = sigtimedwait(&suspend_mask, &suspend_info, &suspend_timeout);

        if (sig == SIGUSR1) { // E2: embark docker is requesting packages
            // E2: read from pipe
            char str_buffer[4] = {0};
            assert(read(fd_embark_request, str_buffer, 4) == 4);
            embark_queue = atoi(str_buffer);
            assert(embark_queue > 0);
            printf("[Storage]\t %d packages requested! (%d available)\n", embark_queue, stored_packages);

            if (stored_packages >= embark_queue) {
                // E3: write to pipe
                stored_packages -= embark_queue;
                assert(write(fd_embark_response, str_buffer, 4) == 4);
                embark_queue = 0;
            }
        } else if (sig == SIGUSR2) { // D3: disembark docker is sending packages
            // D3: read from pipe
            char str_buffer[4] = {0};
            assert(read(fd_disembark_request, str_buffer, 4) == 4);
            disembark_queue = atoi(str_buffer);
            assert(disembark_queue > 0);
            printf("[Storage]\t %d packages received! (%d available)\n", disembark_queue, stored_packages);

            if (stored_packages + disembark_queue <= 20) {
                // D4: notify disembark process that it can continue
                stored_packages += disembark_queue;
                disembark_queue = 0;
                assert(kill(disembark_pid, SIGUSR1) == 0);
            }
        }


        printf("[Storage]\t embark_queue = %d, disembark_queue = %d, stored_packages = %d\n", embark_queue, disembark_queue, stored_packages);

        if (embark_queue > 0) {
            if (stored_packages >= embark_queue) {
                // E3: write to pipe
                stored_packages -= embark_queue;
                char str_buffer[4] = {0};
                sprintf(str_buffer, "%d", embark_queue);
                assert(write(fd_embark_response, str_buffer, 4) == 4);
                printf("[Storage]\t Notifying embark that it can send its package!\n");
                embark_queue = 0;
            } else if (disembark_finished) {
                fprintf(stderr, "No packages in storage and no boat to disembark!\n");
                exit(2);
            }
        }

        if (disembark_queue > 0) {
            if (stored_packages + disembark_queue <= 20) {
                // D4: notify disembark process that it can continue
                stored_packages += disembark_queue;
                disembark_queue = 0;
                assert(kill(disembark_pid, SIGUSR1) == 0);
                printf("[Storage]\t Notifying disembark that it can send its package!\n");
            } else if (embark_finished) {
                fprintf(stderr, "No room in storage and no boat to embark!\n");
                exit(3);
            }
        }

        // Check if the embark process has finished
        if (!embark_finished) {
            int status;
            int pid = waitpid(embark_pid, &status, WNOHANG);
            if (pid == embark_pid && WIFEXITED(status)) {
                embark_finished = true;
                printf("[Storage]\t Embark process has finished with status %d\n", WEXITSTATUS(status));
            } else if (pid == -1) {
                perror("waitpid");
            }
        }

        // Check if the disembark process has finished
        if (!disembark_finished) {
            int status;
            int pid = waitpid(disembark_pid, &status, WNOHANG);
            if (pid == disembark_pid && WIFEXITED(status)) {
                disembark_finished = true;
                printf("[Storage]\t Disembark process has finished with status %d\n", WEXITSTATUS(status));
            } else if (pid == -1) {
                perror("waitpid");
            }
        }
    }
}

int main() {
    // Pipes for communication between the main thread and the docker threads
    int fd_docker_embark[2];
    int fd_docker_disembark[2];
    int fd_storage_embark[2];
    // int fd_storage_disembark[2];

    // Pipes for communication between the boat threads and the docker threads
    int fd_boat_embark[2];
    int fd_boat_disembark[2];

    assert(pipe(fd_docker_embark) == 0);
    assert(pipe(fd_docker_disembark) == 0);
    assert(pipe(fd_storage_embark) == 0);
    // assert(pipe(fd_storage_disembark) == 0);
    assert(pipe(fd_boat_embark) == 0);
    assert(pipe(fd_boat_disembark) == 0);

    const size_t N_BOATS = 50;
    pid_t* boats = malloc(sizeof(pid_t) * N_BOATS);
    for (size_t n = 0; n < N_BOATS; n++) {
        pid_t pid = fork();
        if (pid == 0) {
            boat(fd_boat_embark[0], fd_boat_disembark[1]);
            return 0;
        }
        boats[n] = pid;
    }

    pid_t docker_1 = fork();
    assert(docker_1 >= 0);
    if (docker_1 == 0) {
        docker_embark(
            fd_docker_embark[1],
            fd_storage_embark[0],
            fd_boat_embark[1],
            boats,
            N_BOATS
        );
        return 0;
    }
    pid_t docker_2 = fork();
    assert(docker_2 >= 0);
    if (docker_2 == 0) {
        docker_disembark(
            fd_docker_disembark[1],
            fd_boat_disembark[0],
            boats,
            N_BOATS
        );
        return 0;
    }

    storage(
        fd_docker_embark[0],
        fd_storage_embark[1],
        fd_docker_disembark[0],
        docker_1,
        docker_2
    );
}

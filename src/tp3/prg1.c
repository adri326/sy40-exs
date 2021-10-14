/*-----------------------------------------------------------
               Les tubes ordinaires
               le pere ecrit, le fils lit
-------------------------------------------------------------*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

void erreur(const char *msg) {
    perror(msg);
}

#define LEN 256

int main(void) {
    pid_t pid;
    int tube[2], tube2[2];
    int ret_out, ret_in;
    char *buffer;

    if (pipe(tube) == -1 || pipe(tube2) == -1) {
        erreur("Erreur de creation du pipe");
        exit(EXIT_FAILURE);
    }

    buffer = (char*)malloc(LEN * sizeof(char));

    switch (pid = fork()) {
        case (pid_t) - 1:
            erreur("fork");

        case (pid_t) 0: { // Fils
            close(tube[1]); // Fermeture en ecriture
            close(tube2[0]); // Fermeture en lecture

            int read_status = read(tube[0], buffer, LEN - 1);
            if (read_status == -1) {
                erreur(" Pb Lecture ");
                exit(EXIT_FAILURE);
            }

            printf("[FILS] Je suis le fils de PID %d.\n", getpid());
            printf("[FILS] Lecture dans le tube: '%s'\n", buffer);
            printf("[FILS] Nombre de caractères lus: %d\n", read_status);

            strcat(buffer, " et j'en ressors vivant!");
            int write_status = write(tube2[1], buffer, LEN - 1);
            if (write_status == -1) {
                erreur(" Pb Écriture ");
                exit(EXIT_FAILURE);
            }

            printf("[FILS] Écriture dans le tube 2: '%s'\n");
            printf("[FILS] Nombre de caractères écrits: %d.\n", write_status);

            close(tube[0]);
            close(tube2[1]);

            exit(0);
            break;
        }

        default: // Père
            close(tube[0]); // Fermeture en lecture
            close(tube2[1]); // Fermeture en écriture

            strcpy(buffer, "Je passe dans le tube");

            int write_status = write(tube[1], buffer, LEN - 1);
            if (write_status == -1) {
                erreur(" Pb Écriture ");
                exit(EXIT_FAILURE);
            }

            printf("[PÈRE] Je suis le Pere de PID %d.\n", getpid());
            printf("[PÈRE] Ecriture dans le Tube : '%s'\n", buffer);
            printf("[PÈRE] Taille du buffer %d\n", LEN);

            int read_status = read(tube2[0], buffer, LEN - 1);
            if (read_status == -1) {
                erreur(" Pb Lecture ");
                exit(EXIT_FAILURE);
            }

            printf("[PÈRE] Lecture dans le tube: '%s'\n", buffer);
            printf("[PÈRE] Nombre de caractères lus: %d\n", read_status);

            close(tube[1]);
            close(tube2[0]);

            wait(NULL);
            break;
    }

    free(buffer);
    return EXIT_SUCCESS;
}

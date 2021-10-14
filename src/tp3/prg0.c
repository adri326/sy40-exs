/* Programme à commenter */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (void)
{
    int tube[2];
    unsigned char buffer[256];
    int i;

    fprintf(stdout, "Création du tube\n");
    if (pipe(tube) != 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < 256; i ++) buffer[i] = i;

    fprintf(stdout, "Écriture dans le tube\n");
    if (write(tube[1], buffer, 256) != 256) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Lecture depuis le tube\n");
    if (read(tube[0], buffer, 256) != 256) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Vérification... ");
    for (i = 0; i < 256; i ++)
        if (buffer[i] != i) {
            fprintf(stdout, "Erreur : i = %d, buffer[i] = %d\n", i, buffer[i]);
            exit(EXIT_FAILURE);
        }
    fprintf(stdout, "Ok\n");
    return EXIT_SUCCESS;
}

/*-----------------------------------------------------------
               Les tubes ordinaires
               le pere lit, le fils ecrit
-------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

void erreur(const char* msg) {
    perror(msg);
    exit(1);
}

#define LGMAX 100

#ifdef PARENT

int main() {
    int pfd[2];
    char tampon[LGMAX];

    if (pipe(pfd) == -1) erreur("pipe");

    switch (fork()) {
        case -1:
            erreur("fork");
        default: { // Père
            char* p = tampon;
            close(pfd[1]);
            while (read(pfd[0], p, 1) != 0) p++;
            printf("Chaîne lue dans le tube: %s\n",tampon);
            break;
        }
        case 0: // Fils
            close(pfd[0]);
            sprintf(tampon, "%d", pfd[1]);  /* communique num desc */
            if (execl(BUILD_DIR "/prg2TerFils", "prg1TerFils", tampon, NULL) == -1)
                erreur("execl");
            // Note: !, unreachable
    }
}

#else // PARENT

int main(int argc, char* argv[]) {
    // Retrieve file descriptor from the first argument
    int fd = 1;
    if (argc >= 2) fd = atoi(argv[1]);
    else erreur("Not enough arguments!");

    dprintf(fd, "Hello, ");
    dprintf(fd, "I ");
    dprintf(fd, "am ");
    dprintf(fd, "the ");
    dprintf(fd, "child!\n");

    close(fd);
}

#endif // PARENT

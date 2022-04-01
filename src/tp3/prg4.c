/*-----------------------------------------------------------
               Redirection de l'Entree et la Sortie standard d'un prgm en execution
               Lancement d'une sous routine "wc - w "
-------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/stat.h> // Pour stat

void erreur(const char* msg) {
    perror(msg);
    exit(1);
}

/// We need *two* tubes: one for communication to the child process and
/// one for communication from the child process.
int AppelCommande(const char* commande, int fd[2], int fd2[2]) {
    pipe(fd);
    pipe(fd2);
    pid_t pid = fork();
    if (pid == -1) erreur("Fork");
    else if (pid == 0) { // Fils
        char* buffer = (char*)malloc(sizeof(char) * (strlen(commande) + 1));
        strcpy(buffer, commande);
        assert(dup2(fd[0], STDIN_FILENO) != -1);
        assert(dup2(fd2[1], STDOUT_FILENO) != -1);
        close(fd[1]);
        close(fd2[0]);

        char* split[1024] = {0,};
        char* ptr = strtok(buffer, " ");
        size_t n = 0;

        while (ptr) {
            split[n++] = ptr;
            ptr = strtok(NULL, " ");
        }

        execvp(split[0], split);
    } else {
        close(fd[0]);
        close(fd2[1]);
        // noop
    }

    return 0;
}

int main(int argc, char* argv[]) {
    int tube[2], tube2[2];
    FILE* fichier;
    char* contenu;
    struct stat status;

    char c;

    if (argc != 2) {
        fprintf(stderr, "Syntaxe : %s fichier\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (stat(argv[1], &status) != 0) {
        erreur("stats");

    }

    if ((contenu = malloc(status.st_size)) == NULL)
        erreur("Malloc");
    if ((fichier = fopen(argv[1], "r")) == NULL)
        erreur("fopen");
    if (fread(contenu, 1, status.st_size, fichier) != status.st_size)
        erreur("fread");
    fclose(fichier);

    /* Lancement activite */

    if (AppelCommande("wc -w", tube, tube2) != 0)
        erreur("Invoque processus");

    /* On ecrit */

    write(tube[1], contenu, status.st_size);
    close(tube[1]);

    fprintf(stdout, "Nombre de mots :  ");
    while (read(tube2[0], &c, 1) == 1)
        fputc(c, stdout);
    close(tube2[0]);

    /*************************/

    if (AppelCommande("wc -l", tube, tube2) != 0)
        erreur("Invoque processus");

    /* On ecrit */

    write(tube[1], contenu, status.st_size);
    close(tube[1]);

    fprintf(stdout, "Nombre de lignes :  ");
    while (read(tube2[0], &c, 1) == 1)
        fputc(c, stdout);
    close(tube2[0]);

    /***********************/

    free(contenu);
    return EXIT_SUCCESS;
}

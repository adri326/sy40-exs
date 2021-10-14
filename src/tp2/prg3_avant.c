/*-----------------------------------------------------------
                  recouvrement
-------------------------------------------------------------*/

#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

void attributsProcessus(const char* quand) {
    printf("\n%s\n", quand);
    printf("--------------------------------\n");
    printf("Numero du processus      : %d\n", getpid());
    printf("Numero du processus pere : %d\n", getppid());
    printf("Proprietaire             : %d\n", getuid());
}

int main(void) {
    attributsProcessus("AVANT recouvrement");
    fflush(stdout); /* cf remarque 1 */
    if (execl(BUILD_DIR "/prg3_apres", "prg3_apres", "abc", NULL)) {
        fprintf(stderr, "Couldn't execute " BUILD_DIR "/prg3_apres !\n");
        return 1;
    }
    printf("ne passe pas ici\n");
    // Ce message n'est pas affiché car le `prg3_apres` n'a soit pas pu être lu (dans quel cas le programme s'arrête),
    // soit il a pu être lu et le programme de celui-ci remplace l'entièreté de `prg3_avant`, ce qui inclut le printf.
    return 0;
}

/*--------------------------------------------------------------
   REMARQUE 1:

    meme chose que remarque 2 dans prg3.c
    fflush prudent car stdout peut etre redirige, auquel cas les
    tampons ne sont pas vides et les sorties du premier
    appel de attributsProcessus sont ecrasees
-------------------------------------------------------------*/

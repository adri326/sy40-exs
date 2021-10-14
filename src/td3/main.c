#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    if (argc < 2) exit(1);
    // Create pipe:
    int desc[2]; // [read, write]
    pipe(desc);
    // Fork!
    pid_t pid = fork();

    if (pid == 0) { // Child
        close(desc[1]); // Close unused pipe output
        dup2(desc[0], 0); // Put pipe input at the place of stdin
        close(desc[0]); // Close unused file descriptor
        if (execvp(argv[1], argv+1)) { // Execute given command
            fprintf(stderr, "Couldn't execute '%s ...'!\n", argv[1]);
        }
    } else if (pid > 0) { // Parent
        close(desc[0]); // Close unused pipe input
        dup2(desc[1], 1); // Put pipe output at the place of stdout
        close(desc[1]); // Close unused file descriptor
        if (execlp("ls", "ls", "-l", NULL)) { // Execute ls -l
            fprintf(stderr, "Couldn't execute 'ls -l'!\n");

            // On fail, wait for child (not necessary)
            int status;
            waitpid(pid, &status, 0);
            return 1;
        }
    } else {
        fprintf(stderr, "Couldn't fork()!\n");
        return 1;
    }
}

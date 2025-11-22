/*
 * shittyshell.c
 * A very shitty shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    char *line = NULL;
    size_t bufsize = 0;
    pid_t pid;

    while (1) {
        // 1. Print prompt
        printf("$ ");
        fflush(stdout);

        // 2. Read input line
        ssize_t nread = getline(&line, &bufsize, stdin);
        if (nread == -1) {
            printf("\n");
            break;  // EOF, exit gracefully
        }

        // 3. Trim newline
        if (nread > 0 && line[nread - 1] == '\n') {
            line[nread - 1] = '\0';
        }

        // If user enters empty line, continue
        if (strlen(line) == 0) {
            continue;
        }

        // 4. Fork
        pid = fork();

        if (pid == 0) {
            // ---- CHILD ----

            // 5. Create argument vector
            char *argz[2];
            argz[0] = line;
            argz[1] = NULL;

            // 6. Exec
            if (execv(line, argz) == -1) {
                perror("shittyshell");
                exit(1);
            }

        } else if (pid > 0) {
            // ---- PARENT ----
            wait(NULL);  // 7. Wait for child to finish
        } else {
            perror("fork");
        }
    }

    free(line);
    return 0;
}

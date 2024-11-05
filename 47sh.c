/*
Summary Table for Child Process exits:

    Macro / Flag	Purpose
    WIFEXITED	    Checks if the child exited normally.
    WEXITSTATUS	    Retrieves the exit code of a normally terminated child (only if WIFEXITED is true).
    WIFSIGNALED	    Checks if the child was terminated by a signal.
    WTERMSIG	    Retrieves the signal that terminated the child (only if WIFSIGNALED is true).
    WIFSTOPPED	    Checks if the child was stopped by a signal.
    WSTOPSIG	    Retrieves the signal that stopped the child (only if WIFSTOPPED is true).
    WIFCONTINUED	Checks if a stopped child was resumed.
    WNOHANG	        Makes waitpid() non-blocking, returning immediately if no child state has changed.
    WUNTRACED	    Makes waitpid() return for stopped children (even if not terminated).
    WCONTINUED	    Makes waitpid() return if a stopped child has been resumed (only if continued with SIGCONT).

*/

/*
Summary Table for exec command family:

    Function	Argument Type	Environment	Description
    execl	List of arguments	No	Executes a program with a variable argument list.
    execv	Array of arguments	No	Executes a program with an array of arguments.
    execle	List of arguments and environment	Yes	Executes a program with a variable argument list and specified environment.
    execve	Array of arguments and environment	Yes	Executes a program with an array of arguments and specified environment.
    execlp	List of arguments	No	Executes a program, searching in PATH.
    execvp	Array of arguments	No	Executes a program with an array of arguments, searching in PATH.

*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <readline/readline.h>
#include <sys/wait.h>

#define INITIAL_SIZE 8
#define SIZE_INCREAMENT 4

void freeCommand(char** command) {
    for(int i = 0; command[i] != NULL; i++) free(command[i]);
}

char **getCommand (char *inp) {
    if (!inp || !strlen(inp)) return NULL;

    char **command = malloc(INITIAL_SIZE * sizeof(char *));
    if (!command) return NULL;

    char *sep = " ";
    char *parsed = NULL;
    int ind = 0;
    int capacity = INITIAL_SIZE;

    parsed = strtok(inp, sep);
    while (parsed) {
        if (ind >= capacity) {
            capacity += SIZE_INCREAMENT;
            char **temp = realloc(command, capacity * sizeof(char *));
            if (!temp) {
                freeCommand(command);
                return NULL;
            }
            command = temp;
            if (!command) return NULL;
        }

        command[ind] = strdup(parsed);
        if (!command[ind]) {
            freeCommand(command);
            return NULL;
        }

        ind++;
        parsed = strtok(NULL, sep);
    }

    command[ind] = NULL;
    return command;
}

int cd (char* path) {
    return chdir(path);
}

int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int state;

    char *prompt = getenv("USER");
    if (!prompt) {
        prompt = "user";
    }
    strcat(prompt, "@Wsh> ");
    
    while (1) {
        input = readline(prompt);
        if (!input) break;

        command = getCommand(input);
        if (!command) {
            free(input);
            continue;
        }

        if (!strcmp(command[0], "exit")) exit(0);

        if (!strcmp(command[0], "cd")) {
            if (cd(command[1]) < 1) perror(command[1]);
            continue;
        }

        child_pid = fork();
        if (child_pid < 0) {
            perror("fork failed");
            free(input);
            freeCommand(command);
            continue;
        }

        if (!child_pid) {
            //  Never returns if the call is successful
            execvp(command[0], command);
            printf("error: unknown command\n");
        } else waitpid(child_pid, &state, WUNTRACED);

        free(input);
        freeCommand(command);
    }

    return 0;
}

// Compile & Run: gcc -o 47sh 47sh.c -lreadline && ./47sh

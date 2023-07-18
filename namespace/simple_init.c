#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <wordexp.h>
#include <errno.h>
#include <sys/wait.h>


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)


static int verbose = 0;


static void child_handler(int sig) 
{
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, 
                WNOHANG | WUNTRACED | WCONTINUED)) != 0) {
        if (pid == -1) {    // wait for any child process.
            if (errno == ECHILD) {  // No more children
                break;
            } else {
                perror("waitpid");
            }
        }

        if(verbose) {
            printf("\tinit: SIGCHLD handler: PID %ld terminated\n", (long) pid);
        }
    }
}

static char **expand_words(char *cmd)
{
    char **arg_vec;
    int s;
    wordexp_t pwordexp;

    s = wordexp(cmd, &pwordexp, 0);
    if (s != 0) {
        fprintf(stderr, "Word expanstion failed\n");
        return NULL;
    }

    arg_vec = calloc(pwordexp.we_wordc + 1, sizeof(char *));
    if (arg_vec == NULL)
        errExit("calloc");

    for (s = 0; s < pwordexp.we_wordc; s++) {
        arg_vec[s] = pwordexp.we_wordv[s];
    }
    arg_vec[pwordexp.we_wordc] = NULL;

    return arg_vec;
}

static void usage(char *pname) 
{
    fprintf(stderr, "Usage: %s [-q]\n", pname);
    fprintf(stderr, "\t-v\tProvide verbose logging\n");

    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    struct sigaction sa;
#define CMD_SIZE 10000
    char cmd[CMD_SIZE];
    pid_t pid;
    int opt;
    while ((opt = getopt(argc, argv, "v")) != -1) {
        switch (opt) {
        case 'v': verbose = 1;   break;
        default: usage(argv[0]); break;
        }
    }

    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = child_handler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");
    if (verbose) {
        printf("\tinit: my PID is %ld\n", (long) getpid());
    }

    signal(SIGTTOU, SIG_IGN);

    if (setpgid(0, 0) == -1)
        errExit("setpgid");
    if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
        errExit("tcsetpgrp-child");
    
    while (1) {
        printf("init$ ");
        if (fgets(cmd, CMD_SIZE, stdin) == NULL) {
            if (verbose)
                printf("\tinit: exiting");
            printf("\n");
            exit(EXIT_SUCCESS);
        }
        if (cmd[strlen(cmd) - 1] == '\n')
            cmd[strlen(cmd) - 1] = '\0';
        if (strlen(cmd) == 0)
            continue;

        pid = fork();

        if (pid == -1)
            errExit("fork");
        if (pid == 0) { // child
            char **arg_vec;
            arg_vec = expand_words(cmd);
            if (arg_vec == NULL)
                continue;
            
            if (setpgid(0, 0) == -1)
                errExit("setpgid");
            if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
                errExit("tcsetpgrp-child");
            
            execvp(arg_vec[0], arg_vec);
            errExit("execvp");
        }

        // parent flls through here
        if (verbose)
            printf("\tinit: created child %ld\n", (long) pid); 

        pause();

        if (tcsetpgrp(STDIN_FILENO, getpgrp()) == -1)
            errExit("tcsetpgrp");
    }

    return 0;
}

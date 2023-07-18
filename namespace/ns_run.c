#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/wait.h>


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

static void usage(char *pname) {
    fprintf(stderr, "Usage: %s [-f] [-n /proc/PID/ns/FILE] cmd [arg...]\n",
            pname);
    fprintf(stderr, "\t-f     Execute command in child process\n");
    fprintf(stderr, "\t-n     Join specified namespace\n");

    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int fd, opt, do_fork;
    pid_t pid;

    do_fork = 0;
    while ((opt = getopt(argc, argv, "+fn:")) != -1) {
        switch (opt) {
        case 'n':   // join a namespace
            fd = open(optarg, O_RDONLY);    // get descriptor for namespace
            if (fd == -1)
                errExit("open");
            if (setns(fd, 0) == -1)
                errExit("setns");
            break;
        case 'f':
            do_fork = 1;
            break;
        default:
            usage(argv[0]);
            break;
        }
    }

    if (argc <= optind)
        usage(argv[0]);
    
    if (do_fork) {
        pid = fork();
        if (pid == -1)
            errExit("fork");
        
        if (pid != 0) { // parent
            if (waitpid(-1, NULL, 0) == -1)
                errExit("waitpid");
            exit(EXIT_SUCCESS);
        }
        // Child falls through to code below
    }

    execvp(argv[optind], &argv[optind]);
    errExit("execvp");

    return 0;
}

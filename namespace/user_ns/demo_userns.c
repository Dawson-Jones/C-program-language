#define _GNU_SOURCE
#include <sys/capability.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)


static int child_func(void *arg)
{
    cap_t caps;

    for (;;) {
        printf(
            "eUID = %ld; eGID = %ld;   ",
            // (long) getuid(), (long) getgid()
            (long) geteuid(), (long) getegid()
        );

        caps = cap_get_proc();
        printf("capabilities: %s\n", cap_to_text(caps, NULL));

        if (arg == NULL)
            break;
        
        sleep(5);
    }

    return 0;
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */


int main(int argc, char *argv[])
{
    pid_t pid;

    pid = clone(child_func, child_stack +  STACK_SIZE,
                CLONE_NEWUSER | SIGCHLD, argv[1]);
    if (pid == -1)
        errExit("clone");

    if (waitpid(pid, NULL, 0) == -1)
        errExit("waitpid");

    return 0;
}

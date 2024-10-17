#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>


#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)


static void test_setns(char *pname, int fd)
{
    char path[PATH_MAX];
    ssize_t s;

    s = readlink("/proc/self/ns/user", path, PATH_MAX);
    if (s == -1)
        errExit("readlink");
    
    printf("%s readlink(\"/proc/self/ns/user\") ==> %s\n", pname, path);

    if (setns(fd, CLONE_NEWUSER) == -1)
        printf("%s setns() failed: %s\n", pname, strerror(errno));
    else 
        printf("%s setns() succeeded\n", pname);
}


static int child_func(void *arg)
{
    long fd = (long) arg;
    usleep(100000);

    test_setns("child: ", fd);

    return 0;
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */


int main(int argc, char *argv[]) {
    pid_t child_pid;
    long fd;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s /proc/PID/ns/user\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        errExit("open");
    }

    child_pid = clone(child_func, child_stack + STACK_SIZE,
                    CLONE_NEWUSER | SIGCHLD, (void *) fd);
    if (child_pid == -1) {
        errExit("clone");
    }

    test_setns("parent: ", fd);
    printf("\n");

    if (waitpid(child_pid, NULL, 0) == -1)
        errExit("waitpid");

    return 0;
}

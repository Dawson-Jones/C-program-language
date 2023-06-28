// 创建一个新的进程并把他放到新的namespace中

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/utsname.h>

#define errExit(msg) do {perror(msg); exit(EXIT_FAILURE);} while (0)

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */


static int child_func(void *hostname)
{
    struct utsname uts;

    if (sethostname(hostname, strlen(hostname)) == -1)
        errExit("sethostname");

    if (uname(&uts) == -1)   
        errExit("uname");
    printf("uts.nodename in child:  %s\n", uts.nodename);

    sleep(120);
    return 0;
}


int main(int argc, char *argv[]) {
    pid_t child_pid;
    char *stack;
    char *stackTop;
    struct utsname uts;
    

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <child-hostname>\n", argv[0]);
        return -1;
    }

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");
    
    stackTop = stack + STACK_SIZE;

    child_pid = clone(child_func, stackTop, CLONE_NEWUTS | SIGCHLD, argv[1]);
    if (child_pid == -1)
        errExit("clone");
    printf("clone() returned %jd\n", (intmax_t) child_pid);
    sleep(1);

    if (uname(&uts) == -1)
        errExit("uname");
    printf("uts.nodename in parent: %s\n", uts.nodename);

    if (waitpid(child_pid, NULL, 0) == -1)
        errExit("waitpid");
    printf("child has terninated\n");

    return 0;
}

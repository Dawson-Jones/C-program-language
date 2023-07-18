#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

void sig_handler(int sig) { }


static int child_func(void *arg) {
    printf("child_func(): PID = %ld\n", (long) getpid());
    printf("child_func(): PPID = %ld\n", (long) getppid());

    char *mount_point = arg;

    if (mount_point != NULL) {
        mkdir(mount_point, 0555);   /* create dir for mount point*/
        if (mount("proc", mount_point, "proc", 0, NULL) == -1)
            errExit("mount");
        
        signal(SIGINT, sig_handler);
        printf("Mounting procfs at %s\n", mount_point);
    }

    // execlp("sleep", "sleep", "600", (char *) NULL);
    pause();
    umount(mount_point);
    errExit("execlp");
}

#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];    /* Space for child's stack */

int main(int argc, char *argv[]) {
    pid_t child_pid;
    child_pid = clone(child_func, 
                    child_stack + STACK_SIZE, 
                    /*CLONE_NEWNS | */CLONE_NEWPID | SIGCHLD, 
                    argv[1]
                );
    if (child_pid == -1)
        errExit("clone");
    
    printf("PID returned by clone(): %ld\n", (long) child_pid);
    if (waitpid(child_pid, NULL, 0) == -1)
        errExit("waitpid");

    return 0;
}

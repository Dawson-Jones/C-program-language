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

// void sig_handle(int sig) { }

int main(int argc, char *argv[]) {
    // umount("/proc2");
    // signal(SIGINT, SIG_HOLD);
    pause();
    printf("continue\n");

    return 0;
}

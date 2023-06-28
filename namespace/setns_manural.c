// 将当前进程加入到已有的namespace中

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)


int main(int argc, char const *argv[]) {
    int fd;
    if (argc < 3) {
        fprintf(stderr, "%s /proc/PID/ns/FILE cmd args...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY | O_CLOEXEC);
    if (fd == -1)
        errExit("open");

    if (setns(fd, 0) == -1)
        errExit("setns");
    
    execvp(argv[2], &argv[2]);
    errExit("evecvp");

    return 0;
}



#define _GNU_SOURCE
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>



int main(int argc, char *argv[]) {
    int fd;
    unsigned int seals;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s /proc/PID/fd/FD\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1)
        err(EXIT_FAILURE, "open");
    
    seals = fcntl(fd, F_GET_SEALS);
    if (seals == -1)
        err(EXIT_FAILURE, "fcntl");

    printf("Existing seals:");
    if (seals & F_SEAL_SEAL)
        printf(" F_SEAL_SEAL");
    if (seals & F_SEAL_SHRINK)
        printf(" F_SEAL_SHRINK");
    if (seals & F_SEAL_GROW)
        printf(" F_SEAL_GROW");
    if (seals & F_SEAL_WRITE)
        printf(" F_SEAL_WRITE");
    if (seals & F_SEAL_FUTURE_WRITE)
        printf(" F_SEAL_FUTURE_WRITE");
    printf("\n");

    exit(EXIT_SUCCESS);
}
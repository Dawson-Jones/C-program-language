#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc, char const *argv[]) {
    struct stat statbuf;

    if (argc != 2) {
        fprintf(stderr, "%s <path>\n", argv[0]);
    }

    stat(argv[1], &statbuf);

    printf("Type of file serial numbers: %ld\n", statbuf.st_ino);
    return 0;
}

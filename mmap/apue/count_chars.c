#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>


int main(int argc, char const *argv[]) {
    int i, fd;
    size_t count;
    struct stat statres;
    const char *file;
    char *str;

    if (argc < 2) {
        fprintf(stderr, "Usage...\n");
        exit(EXIT_FAILURE);
    }

    file = argv[1];

    fd = open(file, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    if (fstat(fd, &statres) < 0) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }


    str = mmap(NULL, statres.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (str == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    close(fd);

    for (i = 0; i < statres.st_size; ++i) {
        if (str[i] == 'a')
            count++;
    }

    printf("count: %ld\n", count);

    munmap(str, statres.st_size);


    return 0;
}

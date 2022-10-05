#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


#define BUF_SIZE 100

int main(int argc, char const *argv[])
{
    int fd;
    struct stat sb;
    char *mapped;

    if ((fd = open(argv[1], O_RDWR)) < 0) {
        perror("open");
        return 1;
    }

    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        return 1;
    }

    if ((mapped = (char *) mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    while (1) {
        printf("%s\n", mapped);
        sleep(2);
    };

    close(fd);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>


int main(int argc, char const *argv[]) {
    int fd = open("test.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    // char buf[1024];
    // read(fd, buf, sizeof(buf) / sizeof(char));
    // printf("%s\n", buf);

    const int BUFFER_SIZE = 10;
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];

    struct iovec iov[2];

    iov[0].iov_base = buffer1;
    iov[0].iov_len = BUFFER_SIZE;
    iov[1].iov_base = buffer2;
    iov[1].iov_len = BUFFER_SIZE;

    ssize_t nr = readv(fd, iov, 2);
    if (nr < 0) {
        perror("readv");
        exit(EXIT_FAILURE);
    }

    printf("readv: %ld\n", nr);
    printf("buffer1\n");
    for (int i = 0; i < iov[0].iov_len; ++i) {
        printf("%c", buffer1[i]);
    }
    printf("\n");
    printf("buffer2\n");
    for (int i = 0; i < iov[1].iov_len; ++i) {
        printf("%c", buffer2[i]);
    }
    printf("\n");

    close(fd);
    return 0;
}

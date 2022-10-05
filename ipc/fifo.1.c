#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define _PATH_NAME_ "/tmp/file.tmp"
#define _SIZE_      100


int main(int argc, char const *argv[])
{
    int fd = open(_PATH_NAME_, O_RDONLY);
    if (fd < 0) {
        perror("open\n");
        return 1;
    }

    char buf[_SIZE_];
    while (1) {
        memset(buf, 0, sizeof(buf));
        int ret =read(fd, buf, sizeof(buf));
        if (ret < 0) {
            perror("read");
            break;
        }

        printf("%s\n", buf);
    }
    
    close(fd);
    return 0;
}

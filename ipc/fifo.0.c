#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define _PATH_NAME_ "/tmp/file.tmp"
#define _SIZE_      100

int main(int argc, char const *argv[])
{
    int ret = mkfifo(_PATH_NAME_, S_IFIFO | 0666);
    if (ret == -1) {
        // perror("mkfifo");
        if (errno != EEXIST) {
            fprintf(stderr, "mkfifo error, %d, %s", errno, strerror(errno));
            return 1;
        }
    }

    char buf[_SIZE_];
    memset(buf, 0, sizeof(buf));
    int fd = open(_PATH_NAME_, O_WRONLY);
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        ret = write(fd, buf, strlen(buf));
        if (ret < 0) {
            perror("write\n");
            break;
        }
    }

    close(fd);
    unlink(_PATH_NAME_);
    return 0;
}

#include <stdio.h>
#include <liburing.h>
#include <sys/stat.h>
#include <sys/ioctl.h>


off_t get_file_size(int fd) {
    struct stat st;
    if(fstat(fd, &st) < 0) {
        perror("fstat");
        return -1;
    }
    if (S_ISBLK(st.st_mode)) {
        unsigned long long bytes;
        if (ioctl(fd, BLKGETSIZE64, &bytes) != 0) {
            perror("ioctl");
            return -1;
        }
        return bytes;
    } else if (S_ISREG(st.st_mode))
        return st.st_size;
    return -1;
}

void output_to_console(char *buf, int len) {
    while (len--) {
        fputc(*buf++, stdout);
    }
}

#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <fcntl.h>  
#include <linux/fb.h>  
#include <sys/mman.h>  
#include <sys/ioctl.h>   
  
#define PAGE_SIZE 4096  


int main(int argc, char const *argv[])
{
    int fd, i;
    unsigned char *p_map;

    fd = open("/dev/mymap", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    p_map = (unsigned char *) mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p_map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    for (i = 0; i < 10; ++i) {
        printf("%d\n", p_map[i]);
    }

    munmap(p_map, PAGE_SIZE);
    return 0;
}

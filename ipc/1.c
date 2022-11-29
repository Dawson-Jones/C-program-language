#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>    
#include <fcntl.h>  
#include <sys/mman.h>


int main(int argc, char **argv)    
{    
    int fd;    
    struct stat sb;    
    char *mapped;    
    
    
    /* 打开文件 */    
    if ((fd = open(argv[1], O_RDWR)) < 0) {    
        perror("open");    
    }    
    
    /* 获取文件的属性 */    
    if ((fstat(fd, &sb)) == -1) {    
        perror("fstat");    
    }    
    
    /* 私有文件映射将无法修改文件 */    
    if ((mapped = (char *) mmap(NULL, sb.st_size, PROT_READ |     
                    PROT_WRITE, MAP_SHARED, /*MAP_PRIVATE,*/ fd, 0)) == MAP_FAILED) {    
        perror("mmap");    
    }    
    
    
    /* 修改一个字符 */    
    mapped[0] = 'l';    
     
    /* 映射完后, 关闭文件也可以操纵内存 */    
    close(fd);    
    return 0;    
}  
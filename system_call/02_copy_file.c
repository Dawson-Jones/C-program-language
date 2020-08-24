/*
 * copy a file use system call
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/file.h>
#include <stdio.h>

#define PERMS 0666  // 8 进制的权限,  对于所有者所在组和其他成员均可读可写

void error(char *, ...);

int main(int argc, char *argv[]) {
    int f1, f2, n;
    char buf[BUFSIZ];

    if (argc != 3)
        error("Usage: cp from to");
    
    if ((f1 = open(argv[1], O_RDONLY, 0)) == -1)
        error("cp: can't open %s", argv[1]);

    if ((f2 = creat(argv[2], PERMS)) == -1)
        error("cp: can't create %s, mode %03o", argv[2], PERMS);

    while ((n = read(f1, buf, BUFSIZ)) > 0)     // 读取和写， 都在上一次操作的后面
        if (write(f2, buf, n) != n)
            error("cp: write error on file %s", argv[2]);

    close(f1);
    close(f2);
    return 0;
}



/*
 *  copy input to output
 */


#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/syscall.h>
#include <sys/file.h>
#include <stdio.h>

/* 无缓冲的单字符输入 */
int my_getchar(void) {
    char c;
    return (read(0, &c, 1) == 1) ? (unsigned char) c : EOF;
}

void my_putchar(unsigned char c) {
    write(1, &c, 1);
}

int main() {
    char buf[BUFSIZ];
    int n;      // total nums of read chars
    unsigned char c;

    printf("call once my_getchar.\n");
    c = my_getchar();
    my_putchar(c);
    printf("\n\n\n");

    while ((n = read(0, buf, BUFSIZ)) > 0)
        write(1, buf, n);

    return 0;
}
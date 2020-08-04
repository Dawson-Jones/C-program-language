#include <stdio.h>
#include <string.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len) {
    size_t i;
    for (i = 0; i < len; i++)
        printf(" %.2x", start[i]);
    printf("\n");
}

void show_int(int x) {
    show_bytes((byte_pointer) &x, sizeof(int));
}

int main(int argc, char const *argv[])
{
    // show_int(1<<31);
    const  char *s = "abcdef";
    show_bytes((byte_pointer) s, strlen(s));  // 不能使用 sizeof(s) 这样得到的结果是 8(char * 指针的大小是 8)
    return 0;

}

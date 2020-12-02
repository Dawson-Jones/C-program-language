#include <stdio.h>

typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer s, size_t len) {
    size_t i;
    char pre = *s;

    for (i = 1; i < len; i++) {
        if (s[i] - pre != 1)
            s[i] = pre + 1;

        pre = s[i];
    }

    printf("%s\n", s);
}

int main(int argc, char const *argv[]) {
    char *s = "abcdee";
    show_bytes((byte_pointer) s, sizeof(s));
    return 0;
}

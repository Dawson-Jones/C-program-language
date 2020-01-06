#include <stdio.h>

int main(int argc, char const *argv[])
{
    int i;

    for (i = 1; i < argc; i++)
        // printf("%s%s", argv[i], (i < argc) ? " " : "");
        printf("%s%s", *++argv, (i < argc) ? " " : "");

    return 0;
}

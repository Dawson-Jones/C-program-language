#include <stdio.h>
#include <ctype.h>


int main() {
    int c;

    while ((c = getchar()) != EOF)
        putchar(tolower(c));

    return 0;
}

/*
gcc generate
then
./a.out >test.txt

EOF is ctrl + D
*/
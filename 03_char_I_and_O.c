#include <stdio.h>

int main()
{
    int c;
    while ((c = getchar()) != EOF)
    {
        if (c!='\n'){
        putchar(c);
        printf("\nchar: %c\n", c); // char
        printf("ascii: %d\n", c);  // ascii
        }
    }
    return 0;
}

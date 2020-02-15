#include <stdio.h>

/*
void main()
{
    int c = getchar();
    while (c != EOF)
    {
        putchar(c);
        c = getchar();
    }
}
*/

// ---------------------------

void main()
{
    int c;
    while ((c = getchar()) != EOF)
    {
        putchar(c);
        printf("\nchar: %c\n", c); // char
        printf("ascii: %d\n", c);  // ascii
    }
}
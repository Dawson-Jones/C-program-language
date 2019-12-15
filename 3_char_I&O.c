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
    while ((c = getline()) != EOF)
    {
        putchar(c);
        printf("\nc: %c\n", c);  // char
        printf("d: %d\n", c);  // ascii
    }
}
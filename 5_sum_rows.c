#include <stdio.h>

void main()
{
    int c, nl;
    nl = 0;
    while ((c = getchar()) != EOF)
    {
        if (c == '\n')
            ++nl;
    }
    printf("sum: %d\n", nl);
}
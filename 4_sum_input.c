#include <stdio.h>

void main()
{
    for(double nc = 0; getchar() != EOF; ++nc)
        printf("%.0f\n", nc);
}
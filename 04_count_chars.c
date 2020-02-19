#include <stdio.h>

int main()
{
    int nc;
    for (nc = 0; getchar() != EOF; nc++)
	;
    printf("a total of %d chars\n", nc);
    return 0;
}

#include <stdio.h>

int main()
{
    int nc;
    int c;
    int sp = 0;
    int enter = 0;
    int table = 0;
    for (nc = 0; (c=getchar()) != EOF; nc++){
        switch (c)
        {
        case ' ':
            sp++;
            break;
        case '\n':
            enter++;
            break;
        case '\t':
            table++;
            break;
        default:
            break;
        }
    }
    printf("a total of %d chars, %d enter, %d tables, %d spaces\n", nc, enter, table, sp);
    return 0;
}

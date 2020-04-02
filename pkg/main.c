#include <stdio.h>
#include "add.h"
int main(int argc, char const *argv[])
{
    int a = 6, b=8;
    int c = add(a, b);
    printf("%d\n", c);
    return 0;
}

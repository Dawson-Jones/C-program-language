#include <stdio.h>
#include "add.h"
int main(int argc, char const *argv[])
{
    int a = 2, b=3;
    int c = add(a, b);
    printf("%d\n", c);
    return 0;
}

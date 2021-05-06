#include <stdio.h>

int main(int argc, char const *argv[])
{
    int min = 1<<31;    // -2147483648
    int max = ~(1<<31); // 2147483647

    int test1 = 1<<30;   // 1073741824
    int test2 = ~(1<<30);

    printf("min: %d\nmax: %d\n", min, max);
    printf("test1: %d\t, test2: %d", test1, test2);
    return 0;
}

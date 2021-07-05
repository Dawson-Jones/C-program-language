#include <stdio.h>

/**
 * 有分号的不会被赋值, 只是执行,
 * 最后 x--(100) 被赋值给了变量 x
 */
#define test(p, x) ({ \
*p = 3;               \
x;})


int main() {
    int t = 1;
    int x;
    x = test(&t, 100);
    printf("t: %d, x: %d\n", t, x);
    return 0;
}
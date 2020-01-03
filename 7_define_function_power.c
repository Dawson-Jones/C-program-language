#include <stdio.h>

/*
int power(int m, int n){
    if (n > 1)
        return m * power(m, n-1);
    return m;
}

void main()
{
    int a = 2;
    int b = 5;
    int num = power(a, b);
    printf("%d的%d次幂是: %d\n", a, b, num);
}
*/

// ---------------------------------

// /*
int power(int base, int n);

int main()
{
    for (int i = 0; i < 10; ++i)
        printf("%d %d %d\n", i, power(2, i), power(-3, i));
    return 0;
}

int power(int base, int n)
{
    int i, p;

    p = 1;
    for (i = 1; i <= n; ++i)
        p = p * base;
    return p;
}
// */
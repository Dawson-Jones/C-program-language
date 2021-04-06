// 找到最大的int

#include<stdio.h>

int main(int argc, char const *argv[])
{
    char x;
    x = 1<<7; // -128  最小的负数
    printf("%d\n", x);
    x = ~x;   // 127   最大的正数
    printf("%d\n", x);
    return 0;
}

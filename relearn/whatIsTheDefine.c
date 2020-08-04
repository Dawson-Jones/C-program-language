#include<stdio.h>
#define ADD add

int (*sub)(int, int);
// 这个是可以通过的, 也就是说, define 是在预编译阶段把关键字替换了而已....
int add(int x, int y){return x + y;}

// TODO: complete
int main(int argc, char const *argv[])
{
    sub = &ADD;
    printf("%d\n", (*sub)(2, 4));
    return 0;
}

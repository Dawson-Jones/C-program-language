#include<stdio.h>

// static  全局变量只对本文件有效
// 所以使用 static 可以避免不同文件重名的问题
int fun(){
    static int count = 10;  // 只在第一次初始化
    return count--;
}

int count = 1;

int main(){
    printf("golbal static\n");
    for(;count<=10;++count)
        printf("%d\t\t%d\n", count, fun());
    return 0;
}

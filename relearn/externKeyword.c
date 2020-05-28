#include <stdio.h>

int func();                 // 声明函数隐式声明了 extern, 相当于 extern int func();

int main() {
    func();
    extern int num;         // 和函数声明一样, 声明一个外部的变量, 需要显式声明, 告诉编译器, find num in context
    printf("%d\n", num);
    return 0;
}


int num = 3;

int func() {
    printf("hello, i'm func\n");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double Add (double x, double y){return x + y;}
double Sub(double x, double y){return x - y;}
double Mul(double x, double y){return x * y;}
double Div(double x, double y){return x / y;}

// 具有5个函数指针的数组, 函数需要两个double类型的参数, 返回值为double类型
double (*funcTable[])(double, double) = {Add, Sub, Mul, Div, pow};  // pow 来自math库

// 函数类型定义法
// 类型 func_t 是一个需要两个double类型参数, 返回值类型为double的函数指针
// typedef double (*func_t)(double, double);
// func_t *funcTable[5] = {Add, Sub, Mul, Div, pow};

char *msgTable[5] = {"Sum", "Difference", "Product", "Quotient", "Power"};

double compute(double (*func)(double, double), double x, double y){
    return func(x, y);
}

int main(){
    int i;

    // 索引变量
    double x = 0, y = 0;
    printf("Enter two operands for some arithmetic: \n");
    if (scanf("%lf %lf", &x, &y) != 2){
        printf("Invalid input.\n");
        return 0;
    }
    for (i=0;i<5;i++)
        printf("%10s: %6.2f\n", msgTable[i], (*funcTable[i])(x,y));

    double (*funcPtr)(double, double);
    funcPtr = pow;
    // printf("the result is: %.2f", (*funcPtr)(2,3));
    printf("the result is: %.2f", compute(funcPtr, x, y));
    return 0;
}


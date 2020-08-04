#include <stdio.h>

double Add (double x, double y){return x + y;}
double Mul(double x, double y){return x * y;}
double Sub(double x, double y){return x - y;}

typedef double (*func_t)(double, double);

/* 
第一种方式, wrap 返回一个 func_t 的类型
*/
func_t 
wrap(int x, int y){
    if (x + y < 5) return Add;
    else return Mul;
}

// 第二种, 返回一个函数指针, 该函数指针的返回值是 double 类型, 接收两个 double 类型的参数
double (*wrap2(int x, int y)) (double, double)
{
    return Sub;
}

int 
main(int argc, char const *argv[]){
    func_t func = wrap(1,2);
    printf("5 add 6: %.2f\n", func(5, 6));
    func = wrap(4, 3);
    printf("5 mul 6: %.2f\n", func(5, 6));

    double (*ff)(double, double) = wrap2(2, 3);
    printf("6 sub 5: %.2f\n", ff(6, 5));

    return 0;
}

#include <stdio.h>

double Add (double x, double y){return x + y;}
double Mul(double x, double y){return x * y;}
double Sub(double x, double y){return x - y;}

typedef double (*func_t)(double, double);

func_t 
wrap(int x, int y){
    if (x + y < 5) return Add;
    else return Mul;
}

double (*wrap2(int x, int y))(double, double)
{
    return Sub;
}

int 
main(int argc, char const *argv[]){
    func_t func = wrap(1,2);
    printf("5 add 6: %.2f\n", func(5, 6));
    func = wrap(4, 3);
    printf("5 mul 6: %.2f\n", func(5, 6));

    // ff 
    double (*ff)(double, double) = wrap2(2, 3);
    printf("6 sub 5: %.2f", ff(6, 5));

    return 0;
}

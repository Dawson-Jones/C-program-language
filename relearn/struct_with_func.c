#include<stdio.h>

int add(int x, int y) { return x + y;}
typedef int (*my_sum)(int, int);

typedef struct {
    int     first_param;
    int     second_param;

    // my_sum  func;
    int (*func)(int x, int y);
} test_t;


int main(int argc, char const *argv[])
{
    int x,y;
    if(scanf("%d %d", &x, &y)!=2){
        printf("invilid input\n");
        return 0;
    }
    test_t test = { x, y, add };
    int res = test.func(test.first_param, test.second_param);
    printf("res: %d\n", res);
    return 0;
}

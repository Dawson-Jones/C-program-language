#include <stdio.h>

int main(int argc, char const *argv[])
{
    extern int num;
    void func();
    num = 1;
    printf("file a -> num: %d\n", num);
    func();
    return 0;
}

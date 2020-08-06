#include <stdio.h>

int main(int argc, char const *argv[])
{
    extern int num;
    void func();

    printf("file a -> num: %d\n", num);
    num = 1;
    func();
    return 0;
}

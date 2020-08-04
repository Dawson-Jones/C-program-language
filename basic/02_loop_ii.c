#include <stdio.h>


int main(int argc, char const *argv[])
{
    int i = 1;
    do
    {
        printf("hello, world\n");
    } while (i--);      /* 这里 i-- 是循环两次, --i 是一次, 注意区别 */
    
    return 0;
}

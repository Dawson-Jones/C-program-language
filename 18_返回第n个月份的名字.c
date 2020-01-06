#include <stdio.h>


char *month_name(int n)
{
    static char *name[] = {  // 声明了一组指针, 指针数组
        "Illegal month",
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"};

    return (n < 1 || n > 12) ? name[1] : name[n];
}


int main()
{
    char *str = month_name(1);
    for (;*str != '\0';str++)
        putchar(*str);

    return 0;
}

#include <stdio.h>

void reverse(char *s){
    if (*s != '\0'){
        reverse(s++);
    }
    putchar(*s);

}


int main()
{
    char str[] = "hello, world";
    reverse(str);
    return 0;
}
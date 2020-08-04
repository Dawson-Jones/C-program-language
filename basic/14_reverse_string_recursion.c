#include <stdio.h>

void reverse(char s[], int i)
{
    if (s[i] != '\0')
        reverse(s, ++i);
    putchar(s[i]);
}

int main()
{
    char str[] = "hello, world";
    reverse(str, 0);
    return 0;
}

#include <stdio.h>


void print(char s[], int length){
    for(;length != 0;length--){
        putchar(s[length]);
    }
    putchar(s[length]);
}


int reverse(char s[]){
    int i = 0; 
    while (s[i] != '\0')
        i++;
    return i;
    

}


int main()
{
    char str[] = "hello, world";
    int strlen = reverse(str);
    print(str, strlen);
    return 0;
}

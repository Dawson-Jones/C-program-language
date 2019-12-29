#include <stdio.h>

int str_len(char *r);

int main(){
    int len;
    len = str_len("hello, world");
    printf("长度为: %d\n", len);
    return 0;
}

int str_len(char *s){
    int n = 0;
    for(n = 0; *s != '\0'; s++)
        n++;
    return n;
}
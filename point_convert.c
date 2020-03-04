#include <stdio.h>
#include <stdlib.h>
int main(){
    int a =6336;
    char *b = (char *)&a;
    printf("%d\n",*b);
    return 0;
}

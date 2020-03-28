#include <stdio.h>
#include <stdlib.h>

int *test(){
    int *buffer = (int *)malloc(6);
    if(buffer==0) exit(1);
    for (int i = 0; i < 6; i++)
        buffer[i] = i;
    return buffer;
}

int main(int argc, char const *argv[])
{
    int *list = test();
    for (int i = 0; i < 6; i++)
        printf("%d\n", list[i]);
    free(list);
    return 0;
}

#include<stdio.h>

void convert(){
    float f = 3.14;
    int i = f;
    printf("f: %.2f\n", f);
    printf("f: %d\n", (int) f);
    printf("i: %d\n", i);
}

int main(){
    convert();

    int i = 0;
    int j = 0;
    int k = 1;
    if ((j = k)== 0)
        printf("ifif");
    printf("i: %d, j: %d, k: %d\n", i, j, k);      
    // i: 0, j: 1, k: 1
    // if里面没有被执行, 但是j还是被赋值了

    union test{
        float   f;
        int     i;
        long    l;
    };
    int size = sizeof(union test);
    printf("union size: %d\n", size);  // 8

    struct t_struct{
        int i;
        long l;
    };
    int size_t_struct = sizeof(struct t_struct);
    printf("struct size: %d\n", size_t_struct);  // 16
    // long 是 8, 这里需要强制对齐, 所以导致了 int 也变成了 8;
}


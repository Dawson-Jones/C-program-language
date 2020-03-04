#include<stdio.h>
int main(){
    int i = 0;
    int j = 0;
    int k = 1;
    if ((j = k)== 0)
        printf("ifif");
    printf("i: %d, j: %d, k: %d\n", i, j, k);      
    // i: 0, j: 1, k: 1
    // if里面没有被执行, 但是j还是被赋值了

    union test{
        float f;
        int i;
    };
    int size = sizeof(union test);
    printf("union size: %d\n", size);

    struct t_struct{
        int i;
        long l;
    };
    int size_t_struct = sizeof(struct t_struct);
    printf("struct size: %d\n", size_t_struct);
}


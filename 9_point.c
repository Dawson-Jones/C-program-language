#include <stdio.h>


void swap(int *px, int *py){
    int temp;
    temp = *px; //temp是*px指向的内存的值
    *px = *py;
    *py = temp;
}


int main(){
    int x = 1;
    int y = 2;
    swap(&x, &y);
    printf("x: %d\ny: %d\n", x, y);
    return 0;
}
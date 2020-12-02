#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 这个例子说明了, 在循环中在堆中申请的内存会在每一次的循环结束后释放
int main() {
    char *res[3];
    for (int i = 0; i < 3; ++i) {
        // char temp[100];
        /* 
         * res: dawson
         * res: dawson
         * res: dawson 
         */

        char *temp = (char *) malloc(sizeof(char) * 100);
        /* 
         * res: hello
         * res: world
         * res: dawson
         */
        if (i == 0) {
            strcpy(temp, "hello");
            res[i] = temp;
        }

        if (i == 1) {
            strcpy(temp, "world");
            res[i] = temp;
        }

        if (i == 2) {
            strcpy(temp, "dawson");
            res[i] = temp;
        }
    }

    for (int i = 0; i < 3; ++i) {
        printf("res: %s\n", res[i]);
    }
    return 0;
}
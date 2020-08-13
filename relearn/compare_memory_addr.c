#include <stdio.h>


int num8 = 010;
int num16= 0x10;

int main() {
    printf("sys8: %d\n", num8);
    printf("sys16: %d\n", num16);

    int test_1 = 1;
    int test_2 = 2;

    int *mem_addr1 = &num8;
    int *mem_addr2 = &test_2;
    printf("compare mem_addr: %ld\n", mem_addr1-mem_addr2);     // -35182134367757
    /* 说明就算不是在一起申请的内存， 也是可以做比较的 */
    return 0;
}

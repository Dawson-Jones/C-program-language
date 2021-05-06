#include <stdio.h>


/*
 * 这里如果是 unsigned 则在下面的赋值并转换为 1; 若无 unsigned 则为 -1.
 * 这里其实可间接证明强制类型转换是先判定正负, 然后后面的数字做相应的转换
 */
struct t {
    unsigned char b: 1;
    char c;
    int i;
};


int main() {
    struct t tt = {
            .b = 1,
            .c = 'a',
            .i = 80
    };

    int x = 5;
    x += (int) tt.b;
    printf("res: %d", x);
    return 0;
}

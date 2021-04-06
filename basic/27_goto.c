#include <stdio.h>

int main(int argc, char const *argv[]) {
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            // if (i == 1 && j == 1) {
                // goto errhand;
            // } else {
                 printf("i: %d, j: %d\n", i, j);
            // }
        }
    }

errhand:
    printf("error handler\n");
    return 0;
}

#include <stdio.h>

#include "shape.h"

int main(int argc, char const *argv[]) {
    Shape shape = Shape_new(2, 3);
    printf("shape x = %d, y = %d\n", Shape_getX(shape), Shape_gety(shape));
    Shape_move(shape, 1, 1);
    printf("shape x = %d, y = %d\n", Shape_getX(shape), Shape_gety(shape));

    Shape_delelte(shape);
    return 0;
}

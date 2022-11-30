#include <stdlib.h>
#include "shape.h"


typedef struct shape_s shape_t;
struct shape_s {
    int x;
    int y;
};


Shape Shape_new(int x, int y) {
    shape_t *new = (shape_t *) malloc(sizeof(shape_t));
    new->x = x;
    new->y = y;

    return new;
}

void Shape_move(Shape shape, int dx, int dy) {
    shape_t *cur = (shape_t *) shape;
    
    cur->x += dx;
    cur->y += dy;
}

int Shape_getX(Shape shape) {
    shape_t *cur = (shape_t *) shape;

    return cur->x;
}

int Shape_gety(Shape shape) {
    shape_t *cur = (shape_t *) shape;

    return cur->y;
}

void Shape_delelte(Shape shape) {
    free(shape);
}
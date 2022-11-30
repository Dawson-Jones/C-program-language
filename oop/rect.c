#include "rect.h"

typedef struct rect_s rect_t;
struct rect_s {
    Shape super;

    int width;
    int height;
};


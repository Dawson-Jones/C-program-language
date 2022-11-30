typedef void *Shape;


Shape Shape_new(int x, int y);
void Shape_move(Shape shape, int dx, int dy);
int Shape_getX(Shape shape);
int Shape_gety(Shape shape);
void Shape_delelte(Shape shape);

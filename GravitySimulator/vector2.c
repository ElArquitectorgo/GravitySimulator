#include "vector2.h"
#include <math.h>

float distance(Vector2 a, Vector2 b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

Vector2 sub(Vector2 a, Vector2 b) {
    Vector2 sub = { a.x - b.x, a.y - b.y };
    return sub;
}

float dot(Vector2 a, Vector2 b) {
    return a.x * b.x + a.y * b.y;
}

Vector2 mult(Vector2 a, float scalar) {
    Vector2 mult = { a.x * scalar, a.y * scalar };
    return mult;
}

void normalize(Vector2* v) {
    float scalar = 1 / ((float)sqrt(v->x * v->x + v->y * v->y));
    v->x *= scalar;
    v->y *= scalar;
}

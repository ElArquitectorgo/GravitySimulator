#include "vector2.h"
#include <math.h>
#include <stdio.h>

void add(Vector2* a, Vector2* b) {
    a->x += b->x;
    a->y += b->y;
}

void subs(Vector2* a, Vector2* b) {
    a->x -= b->x;
    a->y -= b->y;
}

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

Vector2 copy(Vector2 v) {
    Vector2 r = { v.x, v.y };
    return r;
}

void print_vector(Vector2* v) {
    printf("%f, %f\n", v->x, v->y);
}

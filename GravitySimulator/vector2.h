#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct {
    float x;
    float y;
} Vector2;

float distance(Vector2 a, Vector2 b);

Vector2 sub(Vector2 a, Vector2 b);

float dot(Vector2 a, Vector2 b);

Vector2 mult(Vector2 a, float scalar);

void normalize(Vector2* v);

void print_vector(Vector2* v);

#endif

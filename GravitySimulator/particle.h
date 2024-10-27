#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector2.h"

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float mass;
    float heat;
} Particle;

#endif

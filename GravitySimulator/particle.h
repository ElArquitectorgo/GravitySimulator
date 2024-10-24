#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector2.h"

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 acc;
    float heat;
} Particle;

#endif

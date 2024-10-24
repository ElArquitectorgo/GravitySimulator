#ifndef QUADTREE_H
#define QUADTREE_H

#include "particle.h"
#include "vector2.h"
#include <stdbool.h>

typedef struct {
    float x, y, w;
    QuadTree* children;
    bool leaf;
    Particle* particle;

    Vector2 center_mass;
    Vector2 center;
    float total_mass;
    int count;
} QuadTree;

QuadTree init_tree(float x, float y, float w);

void split(QuadTree* tree);

int which(QuadTree* tree, Vector2 v);

void insert(QuadTree* tree, Particle* particle);

#endif

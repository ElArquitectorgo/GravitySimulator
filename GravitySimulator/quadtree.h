#ifndef QUADTREE_H
#define QUADTREE_H

#include "quadtree.h"
#include "particle.h"
#include "vector2.h"
#include <stdbool.h>

struct quadTree {
    float x, y, w;
    struct quadTree* children;
    bool leaf;
    Particle* particle;

    Vector2 center_mass;
    Vector2* center;
    float total_mass;
    int count;
};
typedef struct quadTree QuadTree;

QuadTree init_tree(float x, float y, float w);

void split(QuadTree* tree);

int which(QuadTree* tree, Vector2 v);

void insert(QuadTree* tree, Particle* particle);

#endif

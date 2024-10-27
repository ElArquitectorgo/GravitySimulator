#include "quadtree.h"
#include "constants.h"
#include "vector2.h"
#include <stdlib.h>

QuadTree init_tree(float x, float y, float w) {
    QuadTree tree;
    tree.x = x;
    tree.y = y;
    tree.w = w;
    tree.leaf = true;
    tree.particle = NULL;
    tree.children = malloc(4 * sizeof(QuadTree));

    if (tree.children == NULL) {
        perror("Error allocating memory for children.\n");
        exit(EXIT_FAILURE);
    }

    tree.center_mass.x = 0.0f;
    tree.center_mass.y = 0.0f;
    tree.center = NULL;
    tree.total_mass = 0.0f;
    tree.count = 0;

    return tree;
}

void split(QuadTree* tree) {
    float new_w = tree->w * 0.5;
    tree->children[0] = init_tree(tree->x, tree->y, new_w);
    tree->children[1] = init_tree(tree->x + new_w, tree->y, new_w);
    tree->children[2] = init_tree(tree->x, tree->y + new_w, new_w);
    tree->children[3] = init_tree(tree->x + new_w, tree->y + new_w, new_w);
    tree->leaf = false;
}

int which(QuadTree* tree, Vector2 v) {
    float half_width = tree->w * 0.5;

    if (v.x < tree->x + half_width) {
        return (v.y < tree->y + half_width) ? 0 : 2;
    }
    return (v.y < tree->y + half_width) ? 1 : 3;
}

void insert(QuadTree* tree, Particle* b) {
    if (tree->leaf) {
        if (tree->particle != NULL) {
            add(&tree->center_mass, &b->pos);
            tree->total_mass += b->mass;
            tree->count++;

            Particle* a = tree->particle;

            int index_a = which(tree, a->pos);
            int index_b = which(tree, b->pos);

            while (index_a == index_b) {
                split(tree);
                tree = &tree->children[index_a];
                
                index_a = which(tree, a->pos);
                index_b = which(tree, b->pos);

                add(&tree->center_mass, &a->pos);
                add(&tree->center_mass, &b->pos);
                tree->total_mass += a->mass + b->mass;
                tree->count += 2;
            }

            split(tree);
            tree->children[index_a].particle = a;
            tree->children[index_b].particle = b;

            add(&tree->children[index_a].center_mass, &a->pos);
            add(&tree->children[index_b].center_mass, &b->pos);

            tree->children[index_a].total_mass += a->mass;
            tree->children[index_b].total_mass += b->mass;
            
            tree->children[index_a].count++;
            tree->children[index_b].count++;

            tree->particle = NULL;

            return;
        }

        tree->particle = b;

        add(&tree->center_mass, &b->pos);
        tree->total_mass += b->mass;
        tree->count++;

        return;
    }

    add(&tree->center_mass, &b->pos);
    tree->total_mass += b->mass;
    tree->count++;
    insert(&tree->children[which(tree, b->pos)], b);
}
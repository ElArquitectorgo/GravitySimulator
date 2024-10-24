#include "quadtree.h"
#include "constants.h"
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
        perror("Error al asignar memoria para los hijos");
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
            tree->center_mass.x += b->pos.x;
            tree->center_mass.y += b->pos.y;
            tree->total_mass += M;
            tree->count++;

            Particle* a = tree->particle;

            int index_a = which(tree, a->pos);
            int index_b = which(tree, b->pos);

            while (index_a == index_b) {
                split(tree);
                tree = &tree->children[index_a];
                // Habrá que ver si al reasignar tree se me cambia el puntero de 'a' (no creo)
                index_a = which(tree, a->pos);
                index_b = which(tree, b->pos);

                tree->center_mass.x += a->pos.x;
                tree->center_mass.y += a->pos.y;
                tree->center_mass.x += b->pos.x;
                tree->center_mass.x += b->pos.x;
                tree->total_mass += 2 * M;
                tree->count += 2;
            }

            split(tree);
            tree->children[index_a].particle = a;
            tree->children[index_b].particle = b;

            tree->children[index_a].center_mass.x += a->pos.x;
            tree->children[index_a].center_mass.y += a->pos.y;
            tree->children[index_b].center_mass.x += b->pos.x;
            tree->children[index_b].center_mass.y += b->pos.y;

            tree->children[index_a].total_mass += M;
            tree->children[index_b].total_mass += M;
            
            tree->children[index_a].count++;
            tree->children[index_b].count++;

            tree->particle = NULL;

            return;
        }

        tree->particle = b;
        tree->center_mass.x += b->pos.x;
        tree->center_mass.y += b->pos.y;
        tree->total_mass += M;
        tree->count++;

        return;
    }

    tree->center_mass.x += b->pos.x;
    tree->center_mass.y += b->pos.y;
    tree->total_mass += M;
    tree->count++;
    insert(&tree->children[which(tree, b->pos)], b);
}
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_thread.h>

#include "constants.h"
#include "vector2.h"
#include "particle.h"
#include "quadtree.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

enum Shape {
    Spiral,
    Random,
};

Particle* particles = NULL;
int num_particles = 500;

QuadTree root;

void render_particles(SDL_Renderer* renderer, float zoom, Vector2 offset) {
    for (int i = 0; i < num_particles; i++) {
        SDL_Rect rect = { (int)particles[i].pos.x * zoom + offset.x, (int)particles[i].pos.y * zoom + offset.y, R * zoom, R * zoom };

        SDL_SetRenderDrawColor(renderer, particles[i].heat * 5, (1 - particles[i].heat) * 5, 0xff, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void render_drag(SDL_Renderer* renderer, Vector2 a, Vector2 b) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
}

void add_particles(int n, int x, int y, float zoom, Vector2 offset) {
    Particle* new_particles = realloc(particles, (num_particles + n) * sizeof(Particle));
    if (new_particles == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    particles = new_particles;

    for (int i = num_particles; i < num_particles + n; i++) {
        Vector2 pos = { (x + i - num_particles) * 1 / zoom - offset.x / zoom , (y + i - num_particles) * 1 / zoom - offset.y / zoom };
        Vector2 vel = { 0, 0 };
        Vector2 acc = { 0, 0 };
        Particle particle = { pos, vel, acc, 1.0f };
        particles[i] = particle;
    }

    num_particles += n;
}

void add_particle(Vector2 pos, Vector2 vel, float zoom, Vector2 offset) {
    add_particles(1, pos.x, pos.y, zoom, offset);
    particles[num_particles - 1].vel = vel;
}

float get_rand() {
    return (float)rand() / RAND_MAX;
}

void init_particles(enum Shape shape) {
    Particle* new_particles = malloc(num_particles * sizeof(Particle));
    if (new_particles == NULL) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    particles = new_particles;

    for (int i = 0; i < num_particles; i++) {
        Vector2 pos;
        switch (shape) {
        case Spiral:
            pos.x = WINDOW_WIDTH / 2 + 2 * cos(i) * exp(0.3 * i / 10);
            pos.y = WINDOW_WIDTH / 2 + 2 * sin(i) * exp(0.3 * i / 10);
            break;
        case Random:
            pos.x = get_rand() * WINDOW_WIDTH / 2 + WINDOW_WIDTH / 4;
            pos.y = get_rand() * WINDOW_WIDTH / 2 + WINDOW_WIDTH / 4;
            break;
        }
        
        Vector2 vel = { 0, 0 };
        Particle particle = { pos, vel, 1.0f };
        particles[i] = particle;
    }
}

void update_particles(float dt) {
    for (int i = 0; i < num_particles; i++) {
        particles[i].pos.x += particles[i].vel.x * dt;
        particles[i].pos.y += particles[i].vel.y * dt;
        particles[i].heat *= 0.999;
    }
}

void collide(Particle* a, Particle* b, float dist) {
    Vector2 d_pos = sub(a->pos, b->pos);
    normalize(&d_pos);

    float invHeatA = 1 / (a->heat + 1);
    float invHeatB = 1 / (b->heat + 1);
    Vector2 mtd = mult(d_pos, (2 * R - dist) * invHeatA / (invHeatA + invHeatB));
    a->pos.x += mtd.x;
    a->pos.y += mtd.y;

    Vector2 vel_diff = sub(a->vel, b->vel);
    float impact_speed = dot(vel_diff, d_pos);
    a->heat += abs(impact_speed) * 0.1;

    if (impact_speed > 0) return;

    Vector2 force = mult(d_pos, impact_speed * (1 + RESTITUTION) * 0.5);   
    
    a->vel.x -= force.x;
    a->vel.y -= force.y;
}

Vector2 gravity_acc(Vector2 a, Vector2 b, float m) {
    float dist = distance(b, a);
    if (dist <= 4 * R * R) {
        Vector2 v = { 0, 0 };
        return v;
    }
    return mult(sub(b, a), G * m / (dist * sqrt(dist) + 0.01));
}

void gravitate(Particle* p, QuadTree* tree) {
    if (tree->leaf) {
        if (tree->particle == NULL || tree->particle == p) return;
        Vector2 acc = gravity_acc(p->pos, tree->particle->pos, M);
        p->vel.x += acc.x;
        p->vel.y += acc.y;
        return;
    }

    if (tree->center == NULL) {
        tree->center = malloc(sizeof(Vector2));

        if (tree->center == NULL) {
            perror("Error al asignar memoria para el centro");
            exit(EXIT_FAILURE);
        }

        Vector2 v;
        if (tree->count == 0) {
            v.x = WINDOW_WIDTH / 2;
            v.y = WINDOW_HEIGHT / 2;
        }
        else {
            v = mult(tree->center_mass, 1.0f / tree->count);
        }
        *tree->center = v;
    }

    if (tree->w / distance(p->pos, *(tree->center)) < THETA) {
        Vector2 acc = gravity_acc(p->pos, *(tree->center), tree->total_mass);
        p->vel.x += acc.x;
        p->vel.y += acc.y;
        return;
    }

    for (int i = 0; i < 4; i++) {
        gravitate(p, &tree->children[i]);
    }
}

void gravity() {
    for (int i = 0; i < num_particles; i++) {
        gravitate(&particles[i], &root);
    }
}

void _gravity() {
    for (int i = 0; i < num_particles; i++) {
        Particle* a = &particles[i];
        for (int j = 0; j < num_particles; j++) {
            if (i == j) continue;
            Particle* b = &particles[j];

            Vector2 acc = gravity_acc(a->pos, b->pos, M);

            a->acc.x += acc.x;
            a->acc.y += acc.y;
        }
    }
}

void collide_particles() {
    for (int i = 0; i < num_particles; i++) {
        Particle* a = &particles[i];
        for (int j = 0; j < num_particles; j++) {
            if (i == j) continue;
            Particle* b = &particles[j];

            float dist = distance(b->pos, a->pos);

            if (dist < 2 * R) {
                collide(a, b, dist);
            }
        }
    }
}

void free_tree(QuadTree* tree) {
    if (tree->center != NULL) {
        free(tree->center);
        tree->center = NULL;
    }

    if (tree->children != NULL) {
        if (!tree->leaf) {
            for (int i = 0; i < 4; i++) {
                free_tree(&tree->children[i]);
            }
        }
        free(tree->children);
        tree->children = NULL;
    }
}


void construct_tree() {
    float min_x = 100000000000000000;
    float max_x = -100000000000000000;
    float min_y = 100000000000000000;
    float max_y = -100000000000000000;

    for (int i = 0; i < num_particles; i++) {
        min_x = MIN(min_x, particles[i].pos.x);
        max_x = MAX(max_x, particles[i].pos.x);
        min_y = MIN(min_y, particles[i].pos.y);
        max_y = MAX(max_y, particles[i].pos.y);
    }
    free_tree(&root);
    root = init_tree(min_x, min_y, MAX(max_x - min_x, max_y - min_y));

    for (int i = 0; i < num_particles; i++) {
        insert(&root, &particles[i]);
    }
}

int main() {
    enum Shape shape = Spiral;
    init_particles(shape);

    SDL_Window* window;
    SDL_Renderer* renderer;

    int last_frame_time = 0;

    if (SDL_INIT_VIDEO < 0) {
        fprintf(stderr, "ERROR: SDL_INIT_VIDEO");
    }

    window = SDL_CreateWindow(
        "Gravedad",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    if (!window) {
        fprintf(stderr, "ERROR: !window");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        fprintf(stderr, "!renderer");
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    bool quit = false;
    bool right_click = false;
    Vector2 init_pos = { -1000, -1000 };
    Vector2 final_pos = { -1000, -1000 };
    SDL_Event event;

    float zoom = 1.0f;
    Vector2 offset = { 0, 0 };

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_a:
                    offset.x += 10;
                    break;
                case SDLK_d:
                    offset.x -= 10;
                    break;
                case SDLK_w:
                    offset.y += 10;
                    break;
                case SDLK_s:
                    offset.y -= 10;
                    break;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) {
                    zoom *= 1.1f;
                }
                else if (event.wheel.y < 0) {
                    zoom /= 1.1f;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    init_pos.x = event.button.x;
                    init_pos.y = event.button.y;
                    final_pos.x = event.button.x;
                    final_pos.y = event.button.y;
                    right_click = true;
                }
                else {
                    add_particles(10, event.button.x, event.button.y, zoom, offset);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    final_pos.x = event.button.x;
                    final_pos.y = event.button.y;
                    Vector2 res = sub(final_pos, init_pos);
                    add_particle(init_pos, res, zoom, offset);
                    right_click = false;
                }
                break;
            case SDL_MOUSEMOTION:
                if (right_click) {
                    final_pos.x = event.button.x;
                    final_pos.y = event.button.y;
                }
                break;
            }
        }

        float dt = (SDL_GetTicks() - last_frame_time) / 1000.0;
        last_frame_time = SDL_GetTicks();

        SDL_RenderClear(renderer);

        //RENDER LOOP START
        update_particles(dt);
        construct_tree();
        gravity();
        collide_particles();
        render_particles(renderer, zoom, offset);

        if (right_click) {
            render_drag(renderer, final_pos, init_pos);
        }

        //RENDER LOOP END
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
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

// Heavily inspired in William Y. Feng's implementation.
// See https://github.com/womogenes/GravitySim

// Be careful if you increase by a lot num_particles with Spiral or Solar shapes.
// If you use Solar shape make sure to change G constant to something like 0.0001

enum Shape {
    Spiral,
    Sphere,
    Random,
    Solar,
};

Particle* particles = NULL;
int num_particles = 2000;

QuadTree root;
enum Shape shape = Sphere;

void render_particles(SDL_Renderer* renderer, float zoom, Vector2 offset) {
    for (int i = 0; i < num_particles; i++) {
        SDL_Rect rect = { (int)particles[i].pos.x * zoom + offset.x, (int)particles[i].pos.y * zoom + offset.y, particles[i].radius * zoom, particles[i].radius * zoom };

        //SDL_SetRenderDrawColor(renderer, particles[i].heat * 5, (1 - particles[i].heat) * 5, 0xff, 255);
        float g_channel = (abs(particles[i].vel.x) + abs(particles[i].vel.y)) / 150;
        SDL_SetRenderDrawColor(renderer, 255, 186 / (g_channel + 1), 3, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void render_drag(SDL_Renderer* renderer, Vector2 a, Vector2 b) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
}

void render_tree(SDL_Renderer* renderer, QuadTree* tree, float zoom, Vector2 offset) {
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 255);
    if (tree->children != NULL) {
        if (!tree->leaf) {
            for (int i = 0; i < 4; i++) {
                render_tree(renderer, &tree->children[i], zoom, offset);
            }
        }
    }
    SDL_Rect rect = { tree->x * zoom + offset.x, tree->y * zoom + offset.y, tree->w * zoom, tree->w * zoom };
    SDL_RenderDrawRect(renderer, &rect);
}

float get_rand() {
    return (float)rand() / RAND_MAX;
}

void init_particles(enum Shape shape) {
    Particle* new_particles = malloc(num_particles * sizeof(Particle));
    if (new_particles == NULL) {
        fprintf(stderr, "Error allocating memory for particles.\n");
        exit(1);
    }

    particles = new_particles;

    for (int i = 0; i < num_particles; i++) {
        Vector2 pos = { 0, 0 };
        Vector2 vel = { 0, 0 };
        float mass = 1.0f;
        float radius = 5.0f;
        switch (shape) {
        case Spiral:
            pos.x = WINDOW_WIDTH / 2 + 2 * cos(i) * exp(0.3 * i / 10);
            pos.y = WINDOW_WIDTH / 2 + 2 * sin(i) * exp(0.3 * i / 10);
            break;
        case Sphere:
            float angle = get_rand() * 3.1415 * 2;
            float d = get_rand() * WINDOW_WIDTH;
            pos.x = d * cos(angle) + WINDOW_WIDTH / 2;
            pos.y = d * sin(angle) + WINDOW_WIDTH / 2;

            float mag = d * 0.02;
            vel.x = mag * cos(angle + 3.1415 / 2);
            vel.y = mag * sin(angle + 3.1415 / 2);
            break;
        case Random:
            pos.x = get_rand() * WINDOW_WIDTH;
            pos.y = get_rand() * WINDOW_HEIGHT;
            break;
        case Solar:
            pos.x = WINDOW_WIDTH / 2 + i * 50;
            pos.y = WINDOW_HEIGHT / 2;
            vel.x = -5;
            vel.y = 200;
            if (i == 0) { vel.x = 0; vel.y = 0; }
            mass = 100.0f / (i + 1);
            if (i == 0) mass = 10000.0;
            // In the case where we want to see a relation between size and mass, we
            // use natural logarithm to prevent huge bodies.
            radius *= (int)log(mass + 1);
            break;
        }

        Particle particle = { pos, vel, mass, radius, 1.0f };
        particles[i] = particle;
    }
}

void add_particles(int n, int x, int y, float mass, float zoom, Vector2 offset) {
    Particle* new_particles = realloc(particles, (num_particles + n) * sizeof(Particle));
    if (new_particles == NULL) {
        fprintf(stderr, "Error reallocating memory for particles.\n");
        exit(1);
    }

    particles = new_particles;

    for (int i = num_particles; i < num_particles + n; i++) {
        Vector2 pos = { (x + i - num_particles) * 1 / zoom - offset.x / zoom , (y + i - num_particles) * 1 / zoom - offset.y / zoom };
        Vector2 vel = { 0, 0 };
        Particle particle = { pos, vel, mass, R, 1.0f };
        particles[i] = particle;
    }

    num_particles += n;
}

void add_particle(Vector2 pos, Vector2 vel, float zoom, Vector2 offset) {
    add_particles(1, pos.x, pos.y, 1.0f, zoom, offset);
    vel = mult(vel, 5);
    particles[num_particles - 1].vel = vel;
}

void update_particles(float dt) {
    for (int i = 0; i < num_particles; i++) {
        particles[i].pos.x += particles[i].vel.x * dt;
        particles[i].pos.y += particles[i].vel.y * dt;
        particles[i].heat *= 0.999;
    }
}

Vector2 get_center(Particle* p) {
    // Because we are using squares
    Vector2 p_center = { p->pos.x + p->radius / 2, p->pos.y + p->radius / 2 };
    return p_center;
}

void collide(Particle* a, Particle* b, float dist) {
    if (dist > a->radius / 2 + b->radius / 2) return;

    Vector2 d_pos = sub(get_center(&b->pos), get_center(&a->pos));
    normalize(&d_pos);
    float overlap = dist - (a->radius / 2 + b->radius / 2);
    Vector2 mtd = mult(d_pos, overlap * 2);
    add(&a->pos, &mtd);
    subs(&b->pos, &mtd);

    // https://en.wikipedia.org/wiki/Elastic_collision
    dist = a->radius / 2 + b->radius / 2;
    mult(d_pos, dist);

    float impact_speed = dot(sub(b->vel, a->vel), d_pos);
    a->heat += abs(impact_speed) * 0.1;
    if (impact_speed > 0) return;

    float s1 = 2 * b->mass / (a->mass + b->mass);
    float s2 = -2 * a->mass / (a->mass + b->mass);
    Vector2 n_v = mult(d_pos, impact_speed / (dist * dist));
    Vector2 new_v1 = mult(n_v, s1);
    Vector2 new_v2 = mult(n_v, s2);
    add(&a->vel, &new_v1);
    add(&b->vel, &new_v2);
}

void collision(Particle* p, QuadTree* tree) {
    if (tree->leaf) {
        if (tree->particle == NULL || tree->particle == p) return;

        float dist = distance(get_center(p), get_center(tree->particle));
        collide(p, tree->particle, dist);
        return;
    }

    for (int i = 0; i < 4; i++) {
        bool outside = p->pos.x + 2 * p->radius < tree->children[i].x || p->pos.x - 2 * p->radius > tree->children[i].x + tree->children[i].w
            || p->pos.y + 2 * p->radius < tree->children[i].y || p->pos.y - 2 * p->radius > tree->children[i].y + tree->children[i].w;
        if (!outside) {
            collision(p, &tree->children[i]);
        }
    }
}

void collide_particles() {
    for (int i = 0; i < num_particles; i++) {
        Particle* a = &particles[i];
        collision(a, &root);
    }
}

Vector2 gravity_acc(Vector2 a, Vector2 b, float m) {
    float dist = distance(b, a);
    return mult(sub(b, a), G * m / (dist * sqrt(dist)));
}

void gravitate(Particle* p, QuadTree* tree) {
    if (tree->leaf) {
        if (tree->particle == NULL || tree->particle == p) return;

        Vector2 acc = gravity_acc(p->pos, tree->particle->pos, tree->particle->mass);
        add(&p->vel, &acc);
        return;
    }

    if (tree->center == NULL) {
        tree->center = malloc(sizeof(Vector2));

        if (tree->center == NULL) {
            perror("Error allocating memory for tree.center.\n");
            exit(EXIT_FAILURE);
        }

        *tree->center = mult(tree->center_mass, 1.0f / tree->count);
    }

    if (tree->w / distance(p->pos, *(tree->center)) < THETA) {
        Vector2 acc = gravity_acc(p->pos, *(tree->center), tree->total_mass);
        add(&p->vel, &acc);
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

            Vector2 acc = gravity_acc(a->pos, b->pos, b->mass);
            add(&a->vel, &acc);
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
    bool show_grid = false;
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
                case SDLK_k:
                    show_grid = !show_grid;
                }
                break;
            case SDL_MOUSEWHEEL:
                if (event.wheel.y > 0) {
                    zoom *= 1.1f;
                }
                else if (event.wheel.y < 0) {
                    zoom /= 1.1f;
                }
                offset.x = WINDOW_WIDTH / 2 - WINDOW_WIDTH / 2 * zoom;
                offset.y = WINDOW_HEIGHT / 2 - WINDOW_HEIGHT / 2 * zoom;
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
                    add_particles(10, event.button.x, event.button.y, 1.0f, zoom, offset);
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
        //collide_particles();
        render_particles(renderer, zoom, offset);

        if (right_click) {
            render_drag(renderer, final_pos, init_pos);
        }

        if (show_grid) {
            render_tree(renderer, &root, zoom, offset);
        }

        //RENDER LOOP END
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderPresent(renderer);
        //SDL_Delay(1000);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
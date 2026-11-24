// probabilistic cellular automaton with isotropic diffusion update rule

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PICOFB_IMPLEMENTATION
#include "../picofb.h"

#define R1_RADIUS 4
#define R2_RADIUS 8
#define R2_FACTOR 0.00001f

#define DEFECT_PROB 0.9f
#define DEFECT_MAX 0.1f

#define RENDER_R_FACTOR 0.2f
#define RENDER_G_FACTOR 0.6f
#define RENDER_B_FACTOR 1.0f

#define WIDTH 256
#define HEIGHT 256

PICOFB_Window picofb_window={0};

float grid_1[WIDTH][HEIGHT];
float grid_2[WIDTH][HEIGHT];

void init_grid(float grid[WIDTH][HEIGHT]) {
    for (size_t x = 0; x < WIDTH; ++x) for (size_t y = 0; y < HEIGHT; ++y) grid[x][y] = 0.1f;
}

ssize_t emod(ssize_t a, ssize_t b){
    return (a%b + b)%b;
}

float defect_value(float value) {
    if ((float)rand() / RAND_MAX < DEFECT_PROB) return value + (((float)rand() / RAND_MAX) > 0.5f ? 1.0f : -1.0f) * (((float)rand() / RAND_MAX) * DEFECT_MAX);
    return value < 0 ? 0 : (value > 1 ? 1 : value);
}

float avg_in_radius(float grid_from[WIDTH][HEIGHT], ssize_t cx, ssize_t cy, ssize_t radius) {
    float s = 0;
    size_t c = 0;
    for (ssize_t dy = -(radius - 1); dy <= (radius - 1); ++dy) {
        for (ssize_t dx = -(radius - 1); dx <= (radius - 1); ++dx) {
            ssize_t x = emod(cx + dx, WIDTH);
            ssize_t y = emod(cy + dy, HEIGHT);
            if (dx*dx + dy*dy <= radius*radius) {
                s += grid_from[y][x];
                c += 1;
            }
        }
    }
    return (c > 0 ? s / c : 0);
}

void evolve_grid(float grid_from[WIDTH][HEIGHT], float grid_to[WIDTH][HEIGHT]) {
    for (ssize_t cy = 0; cy < HEIGHT; ++cy) {
        for (ssize_t cx = 0; cx < WIDTH; ++cx) {
            float r1_avg  = avg_in_radius(grid_from, cx, cy, R1_RADIUS);
            float r2_avg = avg_in_radius(grid_from, cx, cy, R2_RADIUS);
            float q = r1_avg - R2_FACTOR * r2_avg;
            grid_to[cy][cx] = defect_value(q);
        }
    }
}

void render_grid(float grid[WIDTH][HEIGHT]) {
    float value = 0;
    for (size_t y = 0; y < HEIGHT; ++y) for (size_t x = 0; x < WIDTH; ++x) {
        value = grid[y][x];
        value *= 255.0f;
        uint8_t r = (uint8_t)(value * RENDER_R_FACTOR);
        uint8_t g = (uint8_t)(value * RENDER_G_FACTOR);
        uint8_t b = (uint8_t)(value * RENDER_B_FACTOR);
        picofb_window.frame_buffer[y * WIDTH + x] = PICOFB_color_argb(255, r, g, b);
    }
}


int main(){
    srand((unsigned int)time(NULL));

    if (!PICOFB_init("RANDOM CLOUDS", WIDTH, HEIGHT, &picofb_window)) {printf("picofb init failed\n"); return 1;}

    init_grid(grid_1);
    init_grid(grid_2);

    bool toggle = false;

    while (!picofb_window.quit){
        if (toggle) {
            render_grid(grid_1);
            evolve_grid(grid_1, grid_2);
        }
        else {
            render_grid(grid_2);
            evolve_grid(grid_2, grid_1);
        }
        toggle = !toggle;

        PICOFB_update(&picofb_window);
    }
    PICOFB_cleanup(&picofb_window);
    return 0;
}

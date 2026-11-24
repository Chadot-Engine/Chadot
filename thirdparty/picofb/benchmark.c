#define PICOFB_IMPLEMENTATION
#include "picofb.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

PICOFB_Window picofb_window={0};

static inline void rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color){
    for (uint16_t i = 0; i < h; ++i) for (uint16_t j = 0; j < w; ++j) if ((y + i) < SCREEN_HEIGHT && (x + j) < SCREEN_WIDTH) picofb_window.frame_buffer[(y + i) * SCREEN_WIDTH + (x + j)] = color;
}

static inline void frame_limit(uint16_t target_fps) {
    static struct timespec last = {0};
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (last.tv_sec != 0) {
        double elapsed = (now.tv_sec - last.tv_sec) + (now.tv_nsec - last.tv_nsec) / 1e9;
        double target = 1.0 / (double) target_fps;
        if (elapsed < target) { struct timespec req = {0, (long)((target - elapsed) * 1e9)}; nanosleep(&req, NULL); }
    }
    clock_gettime(CLOCK_MONOTONIC, &last);
}

int main(){
    clock_t start_clock = clock();
    time_t start_time = time(NULL);
    size_t frames = 0;

    if (!PICOFB_init("TEST", SCREEN_WIDTH, SCREEN_HEIGHT, &picofb_window)) {printf("COULD NOT INIT PICOFB WINDOW\n"); return 1;};

    int8_t bounce_rect_delta_x = 1, bounce_rect_delta_y = 1; 
    int16_t bounce_rect_x = 0, bounce_rect_y = 0, control_rect_x = 0, control_rect_y = 0, scroll_y = 0;
    while (!picofb_window.quit && !picofb_window.keyboard[PICOFB_Key_ESC]){
        // bouncing box
        rectangle(bounce_rect_x, bounce_rect_y, 8, 8, 0xFFFFFF00);
        bounce_rect_x += bounce_rect_delta_x;
        bounce_rect_y += bounce_rect_delta_y;
        if (bounce_rect_x > SCREEN_WIDTH - 8 || bounce_rect_x < 0) bounce_rect_delta_x = -bounce_rect_delta_x;
        if (bounce_rect_y > SCREEN_HEIGHT - 8 || bounce_rect_y < 0) bounce_rect_delta_y = -bounce_rect_delta_y;

        // keyboard
        rectangle(control_rect_x, control_rect_y, 8, 8, 0xFF00FFFF);
        if (picofb_window.keyboard[PICOFB_Key_a] && control_rect_x > 0) --control_rect_x;
        if (picofb_window.keyboard[PICOFB_Key_d] && control_rect_x < SCREEN_WIDTH - 8) ++control_rect_x;
        if (picofb_window.keyboard[PICOFB_Key_w] && control_rect_y > 0) --control_rect_y;
        if (picofb_window.keyboard[PICOFB_Key_s] && control_rect_y < SCREEN_HEIGHT - 8) ++control_rect_y;

        // mouse
        rectangle(picofb_window.mouse.x, picofb_window.mouse.y - 8, 8, 8, (picofb_window.mouse.left ? 0xFFFFFFFF : 0xFFFF0000));
        rectangle(picofb_window.mouse.x + 8, picofb_window.mouse.y - 8, 8, 8, (picofb_window.mouse.middle ? 0xFFFFFFFF : 0xFF00FF00));
        rectangle(picofb_window.mouse.x + 16, picofb_window.mouse.y - 8, 8, 8, (picofb_window.mouse.right ? 0xFFFFFFFF : 0xFF0000FF));
        rectangle(SCREEN_WIDTH - 8, scroll_y, 8, 8, 0xFFFFFFFF);
        if (picofb_window.mouse.scroll_delta > 0 && scroll_y > 0) --scroll_y;
        if (picofb_window.mouse.scroll_delta < 0 && scroll_y < SCREEN_HEIGHT - 8) ++scroll_y;
        
        PICOFB_update(&picofb_window);
        for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) picofb_window.frame_buffer[i] = 0xFF000000;

        ++frames;
    }
    PICOFB_cleanup(&picofb_window);

    double clock_secs = (double)(clock() - start_clock)/CLOCKS_PER_SEC;
    double time_secs = (double)(time(NULL) - start_time);
    printf("Frames rendered: %zu\nCPU Time: %.3fs\nReal Time: %.3fs\nCPU FPS: %.2f\nReal FPS: %.2f\n", frames, clock_secs, time_secs, frames / clock_secs, frames / time_secs);
    return 0;
}

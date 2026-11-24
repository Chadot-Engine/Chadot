// animated fire texture from DOOM

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PICOFB_IMPLEMENTATION
#include "../picofb.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

PICOFB_Window picofb_window={0};

static inline void rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color){
    for (uint16_t i = 0; i < h; ++i) for (uint16_t j = 0; j < w; ++j){
        if ((y + i) < SCREEN_HEIGHT && (x + j) < SCREEN_WIDTH) {
            picofb_window.frame_buffer[(y + i) * SCREEN_WIDTH + (x + j)] = color;
        }
    }
}

#define PALETTE_R 255
#define PALETTE_G 128
#define PALETTE_B 0

void spread_fire(size_t from) {
    uint8_t rand_val = rand() % 4;
    uint8_t decay = (rand_val != 0) * 2;
    size_t to = from - SCREEN_WIDTH - rand_val + 1;
    if (to >= SCREEN_WIDTH * SCREEN_HEIGHT) return;
    uint32_t color = picofb_window.frame_buffer[from];
    if ((color >> 24) <= decay) picofb_window.frame_buffer[to] = 0x00000000;
    else {
        uint8_t a = (color >> 24) - decay;
        float t = (float)(a)/255;
        if(t < 0.5f) {
            float s = t * 2.0f;
            picofb_window.frame_buffer[to] = PICOFB_color_argb(a, (uint8_t)(PALETTE_R * s), (uint8_t)(PALETTE_G * s), (uint8_t)(PALETTE_B * s));
        } else {
            float s = (t - 0.5f) * 2.0f;
            picofb_window.frame_buffer[to] = PICOFB_color_argb(a, (uint8_t)(PALETTE_R + (255 - PALETTE_R) * s), (uint8_t)(PALETTE_G + (255 - PALETTE_G) * s), (uint8_t)(PALETTE_B + (255 - PALETTE_B) * s));
        }
    }
}

void make_fire() {
    for(size_t x = 0 ; x < SCREEN_WIDTH; ++x) for (size_t y = 1; y < SCREEN_HEIGHT ; ++y) spread_fire(y * SCREEN_WIDTH + x);
}

int main(){
    srand((unsigned int)time(NULL));

    if (!PICOFB_init("DOOM FIRE", SCREEN_WIDTH, SCREEN_HEIGHT, &picofb_window)) {printf("picofb init failed\n"); return 1;}

    PICOFB_clear(&picofb_window);

    while (!picofb_window.quit){
        for(size_t i = 0; i < SCREEN_WIDTH; i++) picofb_window.frame_buffer[(SCREEN_HEIGHT-1)*SCREEN_WIDTH + i] = 0xFF000000;
        make_fire();
        PICOFB_update(&picofb_window);
        rectangle(picofb_window.mouse.x, picofb_window.mouse.y, 8, 8, 0x00000000);
    }
    PICOFB_cleanup(&picofb_window);
    return 0;
}

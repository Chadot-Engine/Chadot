#define PICOFB_IMPLEMENTATION
#include "picofb.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

int main(){
    // Allocate window
    PICOFB_Window picofb_window={0};

    // Initialize the window
    if (!PICOFB_init("EXAMPLE", SCREEN_WIDTH, SCREEN_HEIGHT, &picofb_window)) {fprintf(stderr, "init failed\n"); return 1;}

    // loop until quit
    while (!picofb_window.quit) {
        // modify frame buffer during main loop
        PICOFB_clear(&picofb_window);
        for (size_t y = 0; y < SCREEN_HEIGHT; ++y) {
            for (size_t x = 0; x < SCREEN_WIDTH; ++x) {
                picofb_window.frame_buffer[y*SCREEN_WIDTH+x] = PICOFB_color_argb(255, x % 256, y % 256, (x*y) % 256);
            }
        }

       // mouse input
       printf("Mouse: %d, %d %d ", picofb_window.mouse.x, picofb_window.mouse.y, picofb_window.mouse.scroll_delta);
       if (picofb_window.mouse.left) printf("Left ");
       if (picofb_window.mouse.middle) printf("Middle ");
       if (picofb_window.mouse.right) printf("Right ");

       // keyboard input (key_pressed() or .keyboard)
       if (PICOFB_key_pressed(&picofb_window, PICOFB_Key_w)) printf("w");
       if (PICOFB_key_pressed(&picofb_window, PICOFB_Key_s)) printf("s");
       if (picofb_window.keyboard[PICOFB_Key_a]) printf("a");
       if (picofb_window.keyboard[PICOFB_Key_d]) printf("d");
       printf("\n");
       if (PICOFB_key_pressed(&picofb_window, PICOFB_Key_q) || PICOFB_key_pressed(&picofb_window, PICOFB_Key_ESC)) break;

        // update the window
        PICOFB_update(&picofb_window);
    }

    // save last frame to ppm
    PICOFB_save_ppm(&picofb_window, "example.ppm");

    // cleanup window
    PICOFB_cleanup(&picofb_window);
    
    return 0;
}

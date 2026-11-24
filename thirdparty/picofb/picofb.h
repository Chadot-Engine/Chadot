/*
           __                       ______   __            __       
          |  \                     /      \ |  \          |  \      
  ______   \$$  _______   ______  |  $$$$$$\| $$____      | $$____  
 /      \ |  \ /       \ /      \ | $$_  \$$| $$    \     | $$    \ 
|  $$$$$$\| $$|  $$$$$$$|  $$$$$$\| $$ \    | $$$$$$$\    | $$$$$$$\
| $$  | $$| $$| $$      | $$  | $$| $$$$    | $$  | $$    | $$  | $$
| $$__/ $$| $$| $$_____ | $$__/ $$| $$      | $$__/ $$ __ | $$  | $$
| $$    $$| $$ \$$     \ \$$    $$| $$      | $$    $$|  \| $$  | $$
| $$$$$$$  \$$  \$$$$$$$  \$$$$$$  \$$       \$$$$$$$  \$$ \$$   \$$
| $$                                                                
| $$                                                                
 \$$                                                                
                                 
*/
// A minimal single-header cross-platform windowing library for displaying a framebuffer on screen with multi input and mouse handling
//================================================================================================================================
// DOCS
//================================================================================================================================
// Guidelines
// - Allocate a zero initialized PICOFB_Window to store state and pass to API
// - use PICOFB_Window.quit to loop
// - See PICOFB_Key for the keyboard input, PICOFB_Mouse for mouse input
// - Define PICOFB_BACKEND_OVERRIDE and then PICOFB_<X11/WAYLAND/WIN32/SDL>_BACKEND to override backend select
//
// API:
// - bool PICOFB_init(const char* window_title, uint16_t width, uint16_t height, PICOFB_Window* picofb_window) - initialize and present a window with a window title string
// - void PICOFB_update(PICOFB_Window* picofb_window)                                                          - updates the window to display the frame buffer and gets user input
// - void PICOFB_cleanup(PICOFB_Window* picofb_window)                                                         - cleanup the window
//
// User visible fields of PICOFB_Window (cross-platform):
//    uint32_t* frame_buffer;
//    bool keyboard[PICOFB_Key_COUNT]; PICOFB_Mouse mouse;
//    bool quit;
//

//================================================================================================================================
// PICOFB
//================================================================================================================================

#ifndef PICOFB_H_
#define PICOFB_H_

#define PICOFB_VERSION "1.0.0"

//================================================================================================================================
// BACKEND SELECT
//================================================================================================================================

#ifndef PICOFB_BACKEND_OVERRIDE

#ifdef __linux__
    #ifdef PICOFB_WAYLAND
        #define PICOFB_WAYLAND_BACKEND
    #else
        #define PICOFB_X11_BACKEND
    #endif
#else
    #ifdef _WIN32
        #define PICOFB_WIN32_BACKEND
    #else
        #define PICOFB_SDL_BACKEND
    #endif
#endif

#endif // !(PICOFB_BACKEND_OVERRIDE)

//================================================================================================================================
// BACKEND
//================================================================================================================================

#include <stdint.h>
#include <stdbool.h>

#ifdef PICOFB_X11_BACKEND
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#ifndef PICOFB_X11_NO_SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
typedef struct {
    uint16_t width, height;
    Display* display;
    Window window;
    XWindowAttributes wa;
    XImage* image;
    GC gc;
    bool shm_on;
    #ifndef PICOFB_X11_NO_SHM
    XShmSegmentInfo shminfo;
    int shm_event_base;
    bool shm_attached;
    #endif
    Atom wm_delete_window;
    XEvent event;
} PICOFB_DontTouch;
#endif

#ifdef PICOFB_WAYLAND_BACKEND
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#ifdef PICOFB_WAYLAND_OLD_MEMFD
    #include <linux/memfd.h>
    #include <sys/syscall.h>
#endif
typedef struct {
    uint16_t width, height;
    struct wl_display* display;
    struct wl_registry* registry;
    struct wl_compositor* compositor;
    struct wl_shm* shm;
    struct wl_surface* surface;
    struct xdg_wm_base* xdg_wm_base;
    struct xdg_surface* xdg_surface;
    struct xdg_toplevel* xdg_toplevel;
    struct wl_seat* seat;
    struct wl_keyboard* kb;
    struct wl_pointer* pointer;
    struct wl_buffer* buffer;
    int shm_fd;
    bool configured;
} PICOFB_DontTouch;
#endif

#ifdef PICOFB_WIN32_BACKEND
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef struct {
    uint16_t width, height;
    HWND hwnd;
    HDC hdc;
    HDC hdc_mem;
    HBITMAP hbitmap;
    HBITMAP hbitmap_old;
    WNDCLASSEX wc;
    MSG msg;
    char class_name_storage[64];
} PICOFB_DontTouch;
#endif

#ifdef PICOFB_SDL_BACKEND
#include <SDL3/SDL.h>
typedef struct {
    uint16_t width, height;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Event event;
} PICOFB_DontTouch;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//================================================================================================================================
// PUBLIC API
//================================================================================================================================

typedef enum {
    PICOFB_Key_UNKNOWN = 0,
    // Letters
    PICOFB_Key_a, PICOFB_Key_b, PICOFB_Key_c, PICOFB_Key_d,
    PICOFB_Key_e, PICOFB_Key_f, PICOFB_Key_g, PICOFB_Key_h,
    PICOFB_Key_i, PICOFB_Key_j, PICOFB_Key_k, PICOFB_Key_l,
    PICOFB_Key_m, PICOFB_Key_n, PICOFB_Key_o, PICOFB_Key_p,
    PICOFB_Key_q, PICOFB_Key_r, PICOFB_Key_s, PICOFB_Key_t,
    PICOFB_Key_u, PICOFB_Key_v, PICOFB_Key_w, PICOFB_Key_x,
    PICOFB_Key_y, PICOFB_Key_z,
    // Numbers
    PICOFB_Key_0, PICOFB_Key_1, PICOFB_Key_2, PICOFB_Key_3,
    PICOFB_Key_4, PICOFB_Key_5, PICOFB_Key_6, PICOFB_Key_7,
    PICOFB_Key_8, PICOFB_Key_9,
    // Function keys
    PICOFB_Key_F1, PICOFB_Key_F2, PICOFB_Key_F3, PICOFB_Key_F4,
    PICOFB_Key_F5, PICOFB_Key_F6, PICOFB_Key_F7, PICOFB_Key_F8,
    PICOFB_Key_F9, PICOFB_Key_F10, PICOFB_Key_F11, PICOFB_Key_F12,
    // Arrow keys
    PICOFB_Key_UP, PICOFB_Key_DOWN, PICOFB_Key_LEFT, PICOFB_Key_RIGHT,
    // Control keys
    PICOFB_Key_ESC, PICOFB_Key_SPACE, 
    PICOFB_Key_ENTER, PICOFB_Key_TAB, PICOFB_Key_BACKSPACE, PICOFB_Key_DELETE, 
    PICOFB_Key_INSERT, PICOFB_Key_HOME, PICOFB_Key_END, PICOFB_Key_PAGEUP, PICOFB_Key_PAGEDOWN,
    // Modifiers
    PICOFB_Key_LSHIFT, PICOFB_Key_RSHIFT, 
    PICOFB_Key_LCTRL, PICOFB_Key_RCTRL,
    PICOFB_Key_LALT, PICOFB_Key_RALT, 
    PICOFB_Key_LSUPER, PICOFB_Key_RSUPER,
    // Punctuation/Symbols
    PICOFB_Key_MINUS, PICOFB_Key_EQUALS, 
    PICOFB_Key_LBRACKET, PICOFB_Key_RBRACKET, PICOFB_Key_BACKSLASH, 
    PICOFB_Key_SEMICOLON, PICOFB_Key_APOSTROPHE,
    PICOFB_Key_COMMA, PICOFB_Key_PERIOD, PICOFB_Key_SLASH, PICOFB_Key_GRAVE,
    // Lock keys
    PICOFB_Key_CAPSLOCK,
    PICOFB_Key_COUNT
} PICOFB_Key;

typedef struct {
    uint16_t x, y; 
    bool left, middle, right; int8_t scroll_delta;
} PICOFB_Mouse;

typedef struct {
    uint32_t* frame_buffer;
    bool keyboard[PICOFB_Key_COUNT]; PICOFB_Mouse mouse;
    bool quit;
    PICOFB_DontTouch dont_touch;
} PICOFB_Window;

#ifndef PICOFB_DEF
#define PICOFB_DEF static inline
#endif

PICOFB_DEF bool PICOFB_init(const char* window_title, uint16_t width, uint16_t height, PICOFB_Window* picofb_window);
PICOFB_DEF void PICOFB_update(PICOFB_Window* picofb_window);
PICOFB_DEF void PICOFB_cleanup(PICOFB_Window* picofb_window);

//================================================================================================================================
// BACKEND IMPLEMENTATIONS
//================================================================================================================================

#ifdef PICOFB_IMPLEMENTATION

//================================================================================================================================

#ifdef PICOFB_X11_BACKEND

static inline PICOFB_Key PICOFB_X11_itt(KeySym key){
    switch(key) {
        case XK_a: return PICOFB_Key_a;
        case XK_b: return PICOFB_Key_b;
        case XK_c: return PICOFB_Key_c;
        case XK_d: return PICOFB_Key_d;
        case XK_e: return PICOFB_Key_e;
        case XK_f: return PICOFB_Key_f;
        case XK_g: return PICOFB_Key_g;
        case XK_h: return PICOFB_Key_h;
        case XK_i: return PICOFB_Key_i;
        case XK_j: return PICOFB_Key_j;
        case XK_k: return PICOFB_Key_k;
        case XK_l: return PICOFB_Key_l;
        case XK_m: return PICOFB_Key_m;
        case XK_n: return PICOFB_Key_n;
        case XK_o: return PICOFB_Key_o;
        case XK_p: return PICOFB_Key_p;
        case XK_q: return PICOFB_Key_q;
        case XK_r: return PICOFB_Key_r;
        case XK_s: return PICOFB_Key_s;
        case XK_t: return PICOFB_Key_t;
        case XK_u: return PICOFB_Key_u;
        case XK_v: return PICOFB_Key_v;
        case XK_w: return PICOFB_Key_w;
        case XK_x: return PICOFB_Key_x;
        case XK_y: return PICOFB_Key_y;
        case XK_z: return PICOFB_Key_z;
        case XK_0: return PICOFB_Key_0;
        case XK_1: return PICOFB_Key_1;
        case XK_2: return PICOFB_Key_2;
        case XK_3: return PICOFB_Key_3;
        case XK_4: return PICOFB_Key_4;
        case XK_5: return PICOFB_Key_5;
        case XK_6: return PICOFB_Key_6;
        case XK_7: return PICOFB_Key_7;
        case XK_8: return PICOFB_Key_8;
        case XK_9: return PICOFB_Key_9;
        case XK_F1: return PICOFB_Key_F1;
        case XK_F2: return PICOFB_Key_F2;
        case XK_F3: return PICOFB_Key_F3;
        case XK_F4: return PICOFB_Key_F4;
        case XK_F5: return PICOFB_Key_F5;
        case XK_F6: return PICOFB_Key_F6;
        case XK_F7: return PICOFB_Key_F7;
        case XK_F8: return PICOFB_Key_F8;
        case XK_F9: return PICOFB_Key_F9;
        case XK_F10: return PICOFB_Key_F10;
        case XK_F11: return PICOFB_Key_F11;
        case XK_F12: return PICOFB_Key_F12;
        case XK_Up: return PICOFB_Key_UP;
        case XK_Down: return PICOFB_Key_DOWN;
        case XK_Left: return PICOFB_Key_LEFT;
        case XK_Right: return PICOFB_Key_RIGHT;
        case XK_Escape: return PICOFB_Key_ESC;
        case XK_space: return PICOFB_Key_SPACE;
        case XK_Return: return PICOFB_Key_ENTER;
        case XK_Tab: return PICOFB_Key_TAB;
        case XK_BackSpace: return PICOFB_Key_BACKSPACE;
        case XK_Delete: return PICOFB_Key_DELETE;
        case XK_Insert: return PICOFB_Key_INSERT;
        case XK_Home: return PICOFB_Key_HOME;
        case XK_End: return PICOFB_Key_END;
        case XK_Page_Up: return PICOFB_Key_PAGEUP;
        case XK_Page_Down: return PICOFB_Key_PAGEDOWN;
        case XK_Shift_L: return PICOFB_Key_LSHIFT;
        case XK_Shift_R: return PICOFB_Key_RSHIFT;
        case XK_Control_L: return PICOFB_Key_LCTRL;
        case XK_Control_R: return PICOFB_Key_RCTRL;
        case XK_Alt_L: return PICOFB_Key_LALT;
        case XK_Alt_R: return PICOFB_Key_RALT;
        case XK_Super_L: return PICOFB_Key_LSUPER;
        case XK_Super_R: return PICOFB_Key_RSUPER;
        case XK_minus: return PICOFB_Key_MINUS;
        case XK_equal: return PICOFB_Key_EQUALS;
        case XK_bracketleft: return PICOFB_Key_LBRACKET;
        case XK_bracketright: return PICOFB_Key_RBRACKET;
        case XK_backslash: return PICOFB_Key_BACKSLASH;
        case XK_semicolon: return PICOFB_Key_SEMICOLON;
        case XK_apostrophe: return PICOFB_Key_APOSTROPHE;
        case XK_comma: return PICOFB_Key_COMMA;
        case XK_period: return PICOFB_Key_PERIOD;
        case XK_slash: return PICOFB_Key_SLASH;
        case XK_grave: return PICOFB_Key_GRAVE;
        case XK_Caps_Lock: return PICOFB_Key_CAPSLOCK;
        default: return PICOFB_Key_UNKNOWN;
    }
}
static int PICOFB_X11_destroy_image(XImage *img) {(void)img; return 0;}
#ifndef PICOFB_X11_NO_SHM
static Bool PICOFB_X11_shm_pred(Display* d, XEvent* ev, XPointer arg) {
    (void)d;
    return ev->type == *(int*)arg + ShmCompletion;
}
#endif

PICOFB_DEF bool PICOFB_init(const char* window_title, uint16_t width, uint16_t height, PICOFB_Window* picofb_window) {
    if (!picofb_window) return false;
    memset(picofb_window, 0, sizeof(PICOFB_Window));
    picofb_window->dont_touch.width = width; picofb_window->dont_touch.height = height; 
    picofb_window->dont_touch.display = XOpenDisplay(NULL);
    if (picofb_window->dont_touch.display == NULL) {PICOFB_cleanup(picofb_window); return false;}
	int screen = DefaultScreen(picofb_window->dont_touch.display);
	int x = (DisplayWidth(picofb_window->dont_touch.display, screen) - width) / 2;
	int y = (DisplayHeight(picofb_window->dont_touch.display, screen) - height) / 2;
	picofb_window->dont_touch.window = XCreateSimpleWindow(picofb_window->dont_touch.display, XDefaultRootWindow(picofb_window->dont_touch.display), x, y, width, height, 0, 0, 0);
    if (!XGetWindowAttributes(picofb_window->dont_touch.display, picofb_window->dont_touch.window, &picofb_window->dont_touch.wa)) {PICOFB_cleanup(picofb_window); return false;}
    if (picofb_window->dont_touch.wa.depth != 24 && picofb_window->dont_touch.wa.depth != 32) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.gc = XCreateGC(picofb_window->dont_touch.display, picofb_window->dont_touch.window, 0, NULL);
    if (!picofb_window->dont_touch.gc) {PICOFB_cleanup(picofb_window); return false;}
    #ifndef PICOFB_X11_NO_SHM
    if (XShmQueryExtension(picofb_window->dont_touch.display)) {
        picofb_window->dont_touch.shm_on = true;
        picofb_window->dont_touch.image = XShmCreateImage(picofb_window->dont_touch.display, picofb_window->dont_touch.wa.visual, picofb_window->dont_touch.wa.depth, ZPixmap, NULL, &picofb_window->dont_touch.shminfo, width, height);
        if (!picofb_window->dont_touch.image) {PICOFB_cleanup(picofb_window); return false;}
        picofb_window->dont_touch.image->f.destroy_image = PICOFB_X11_destroy_image;
        picofb_window->dont_touch.shminfo.shmid = shmget(IPC_PRIVATE, picofb_window->dont_touch.image->bytes_per_line * picofb_window->dont_touch.image->height, IPC_CREAT|0600);
        if (picofb_window->dont_touch.shminfo.shmid < 0) {PICOFB_cleanup(picofb_window); return false;}
        picofb_window->dont_touch.shminfo.shmaddr = picofb_window->dont_touch.image->data = shmat(picofb_window->dont_touch.shminfo.shmid, 0, 0); 
        if (picofb_window->dont_touch.shminfo.shmaddr == (void*)-1) {PICOFB_cleanup(picofb_window); return false;}
        picofb_window->dont_touch.shminfo.readOnly = False;
        XShmAttach(picofb_window->dont_touch.display, &picofb_window->dont_touch.shminfo);
        picofb_window->dont_touch.shm_attached = true;
        picofb_window->dont_touch.shm_event_base = XShmGetEventBase(picofb_window->dont_touch.display);
        picofb_window->frame_buffer = (uint32_t*)picofb_window->dont_touch.shminfo.shmaddr;
    }
    else {
    #endif
        picofb_window->frame_buffer = calloc(width * height, sizeof(uint32_t));
        if (!picofb_window->frame_buffer) {PICOFB_cleanup(picofb_window); return false;}
        picofb_window->dont_touch.image = XCreateImage(picofb_window->dont_touch.display, picofb_window->dont_touch.wa.visual, picofb_window->dont_touch.wa.depth, ZPixmap, 0, (char*) picofb_window->frame_buffer, width, height, 32, width * sizeof(uint32_t));
        if (!picofb_window->dont_touch.image) {PICOFB_cleanup(picofb_window); return false;}
        picofb_window->dont_touch.image->f.destroy_image = PICOFB_X11_destroy_image;
    #ifndef PICOFB_X11_NO_SHM
    }
    #endif
    picofb_window->dont_touch.wm_delete_window = XInternAtom(picofb_window->dont_touch.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(picofb_window->dont_touch.display, picofb_window->dont_touch.window, &picofb_window->dont_touch.wm_delete_window, 1);
    XSelectInput(picofb_window->dont_touch.display, picofb_window->dont_touch.window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask);
    XStoreName(picofb_window->dont_touch.display, picofb_window->dont_touch.window, window_title ? window_title : "");
    XMapWindow(picofb_window->dont_touch.display, picofb_window->dont_touch.window);
    XSizeHints hints = {0};
    hints.flags = PMinSize | PMaxSize;
    hints.min_width = hints.max_width = width;
    hints.min_height = hints.max_height = height;
    XSetWMNormalHints(picofb_window->dont_touch.display, picofb_window->dont_touch.window, &hints);
    XkbSetDetectableAutoRepeat(picofb_window->dont_touch.display, True, NULL);
    return true;
}

PICOFB_DEF void PICOFB_update(PICOFB_Window* picofb_window) {
    picofb_window->mouse.scroll_delta = 0;
    while (XPending(picofb_window->dont_touch.display) > 0) {
        XNextEvent(picofb_window->dont_touch.display, &picofb_window->dont_touch.event);
        switch(picofb_window->dont_touch.event.type) {
            case KeyPress: {
                PICOFB_Key key = PICOFB_X11_itt(XLookupKeysym(&picofb_window->dont_touch.event.xkey, 0));
                if (key != PICOFB_Key_UNKNOWN) picofb_window->keyboard[key] = true;
                break;
            }
            case KeyRelease: {
                PICOFB_Key key = PICOFB_X11_itt(XLookupKeysym(&picofb_window->dont_touch.event.xkey, 0));
                if (key != PICOFB_Key_UNKNOWN) picofb_window->keyboard[key] = false;
                break;
            }
            case FocusOut:
                memset(picofb_window->keyboard, 0, sizeof(picofb_window->keyboard));
                break;
            case ConfigureNotify:
                if (picofb_window->dont_touch.event.xconfigure.width != picofb_window->dont_touch.width || picofb_window->dont_touch.event.xconfigure.height != picofb_window->dont_touch.height) picofb_window->quit = true;
                break;
            case ButtonPress:
                if (picofb_window->dont_touch.event.xbutton.button == Button1) picofb_window->mouse.left = true;
                if (picofb_window->dont_touch.event.xbutton.button == Button3) picofb_window->mouse.right = true;
                if (picofb_window->dont_touch.event.xbutton.button == Button2) picofb_window->mouse.middle = true;
                if (picofb_window->dont_touch.event.xbutton.button == Button4) picofb_window->mouse.scroll_delta = 1;
                if (picofb_window->dont_touch.event.xbutton.button == Button5) picofb_window->mouse.scroll_delta = -1;
                break;
            case ButtonRelease:
                if (picofb_window->dont_touch.event.xbutton.button == Button1) picofb_window->mouse.left = false;
                if (picofb_window->dont_touch.event.xbutton.button == Button3) picofb_window->mouse.right = false;
                if (picofb_window->dont_touch.event.xbutton.button == Button2) picofb_window->mouse.middle = false;
                break;
            case MotionNotify:
                picofb_window->mouse.x = picofb_window->dont_touch.event.xmotion.x;
                picofb_window->mouse.y = picofb_window->dont_touch.event.xmotion.y;
                break;
            case ClientMessage:
                if ((Atom) picofb_window->dont_touch.event.xclient.data.l[0] == picofb_window->dont_touch.wm_delete_window) picofb_window->quit = true;
            break;
        }
    }
    #ifndef PICOFB_X11_NO_SHM
    if (picofb_window->dont_touch.shm_on) {
        XShmPutImage(picofb_window->dont_touch.display, picofb_window->dont_touch.window, picofb_window->dont_touch.gc, picofb_window->dont_touch.image, 0, 0, 0, 0, picofb_window->dont_touch.width, picofb_window->dont_touch.height, True);
        XIfEvent(picofb_window->dont_touch.display, &picofb_window->dont_touch.event, PICOFB_X11_shm_pred, (XPointer)&picofb_window->dont_touch.shm_event_base);
    }
    else {
    #endif
        XPutImage(picofb_window->dont_touch.display, picofb_window->dont_touch.window, picofb_window->dont_touch.gc, picofb_window->dont_touch.image, 0, 0, 0, 0, picofb_window->dont_touch.width, picofb_window->dont_touch.height);
    #ifndef PICOFB_X11_NO_SHM
    }
    #endif    
    XFlush(picofb_window->dont_touch.display);
}

PICOFB_DEF void PICOFB_cleanup(PICOFB_Window* picofb_window) {
    #ifndef PICOFB_X11_NO_SHM
    if (picofb_window->dont_touch.shm_attached) XShmDetach(picofb_window->dont_touch.display, &picofb_window->dont_touch.shminfo);
    #endif
    if (picofb_window->dont_touch.image) XDestroyImage(picofb_window->dont_touch.image);
    #ifndef PICOFB_X11_NO_SHM
    if (picofb_window->dont_touch.shm_on) {
        shmdt(picofb_window->dont_touch.shminfo.shmaddr);
        shmctl(picofb_window->dont_touch.shminfo.shmid, IPC_RMID, 0);
    }
    #endif
    if (picofb_window->dont_touch.gc) XFreeGC(picofb_window->dont_touch.display, picofb_window->dont_touch.gc);
    if (picofb_window->dont_touch.window) XDestroyWindow(picofb_window->dont_touch.display, picofb_window->dont_touch.window);
    if (picofb_window->dont_touch.display) XCloseDisplay(picofb_window->dont_touch.display);
    if (picofb_window->frame_buffer && !picofb_window->dont_touch.shm_on) free(picofb_window->frame_buffer);
    memset(picofb_window, 0, sizeof(PICOFB_Window));
}

#endif // PICOFB_X11_BACKEND

//================================================================================================================================

#ifdef PICOFB_WAYLAND_BACKEND

static inline PICOFB_Key PICOFB_WAYLAND_itt(uint32_t keycode) {
    switch(keycode) {
        case KEY_A: return PICOFB_Key_a; 
        case KEY_B: return PICOFB_Key_b; 
        case KEY_C: return PICOFB_Key_c; 
        case KEY_D: return PICOFB_Key_d;
        case KEY_E: return PICOFB_Key_e; 
        case KEY_F: return PICOFB_Key_f; 
        case KEY_G: return PICOFB_Key_g; 
        case KEY_H: return PICOFB_Key_h;
        case KEY_I: return PICOFB_Key_i; 
        case KEY_J: return PICOFB_Key_j; 
        case KEY_K: return PICOFB_Key_k; 
        case KEY_L: return PICOFB_Key_l;
        case KEY_M: return PICOFB_Key_m; 
        case KEY_N: return PICOFB_Key_n;
        case KEY_O: return PICOFB_Key_o; 
        case KEY_P: return PICOFB_Key_p;
        case KEY_Q: return PICOFB_Key_q; 
        case KEY_R: return PICOFB_Key_r;
        case KEY_S: return PICOFB_Key_s; 
        case KEY_T: return PICOFB_Key_t;
        case KEY_U: return PICOFB_Key_u; 
        case KEY_V: return PICOFB_Key_v;
        case KEY_W: return PICOFB_Key_w; 
        case KEY_X: return PICOFB_Key_x;
        case KEY_Y: return PICOFB_Key_y; 
        case KEY_Z: return PICOFB_Key_z;
        case KEY_0: return PICOFB_Key_0; 
        case KEY_1: return PICOFB_Key_1;
        case KEY_2: return PICOFB_Key_2; 
        case KEY_3: return PICOFB_Key_3;
        case KEY_4: return PICOFB_Key_4; 
        case KEY_5: return PICOFB_Key_5;
        case KEY_6: return PICOFB_Key_6; 
        case KEY_7: return PICOFB_Key_7;
        case KEY_8: return PICOFB_Key_8; 
        case KEY_9: return PICOFB_Key_9;
        case KEY_F1: return PICOFB_Key_F1; 
        case KEY_F2: return PICOFB_Key_F2; 
        case KEY_F3: return PICOFB_Key_F3; 
        case KEY_F4: return PICOFB_Key_F4;
        case KEY_F5: return PICOFB_Key_F5; 
        case KEY_F6: return PICOFB_Key_F6; 
        case KEY_F7: return PICOFB_Key_F7; 
        case KEY_F8: return PICOFB_Key_F8;
        case KEY_F9: return PICOFB_Key_F9; 
        case KEY_F10: return PICOFB_Key_F10; 
        case KEY_F11: return PICOFB_Key_F11; 
        case KEY_F12: return PICOFB_Key_F12;
        case KEY_UP: return PICOFB_Key_UP; 
        case KEY_DOWN: return PICOFB_Key_DOWN; 
        case KEY_LEFT: return PICOFB_Key_LEFT; 
        case KEY_RIGHT: return PICOFB_Key_RIGHT;
        case KEY_ESC: return PICOFB_Key_ESC; 
        case KEY_SPACE: return PICOFB_Key_SPACE; 
        case KEY_ENTER: return PICOFB_Key_ENTER; 
        case KEY_TAB: return PICOFB_Key_TAB;
        case KEY_BACKSPACE: return PICOFB_Key_BACKSPACE; 
        case KEY_DELETE: return PICOFB_Key_DELETE; 
        case KEY_INSERT: return PICOFB_Key_INSERT;
        case KEY_HOME: return PICOFB_Key_HOME; 
        case KEY_END: return PICOFB_Key_END; 
        case KEY_PAGEUP: return PICOFB_Key_PAGEUP; 
        case KEY_PAGEDOWN: return PICOFB_Key_PAGEDOWN;
        case KEY_LEFTSHIFT: return PICOFB_Key_LSHIFT; 
        case KEY_RIGHTSHIFT: return PICOFB_Key_RSHIFT;
        case KEY_LEFTCTRL: return PICOFB_Key_LCTRL; 
        case KEY_RIGHTCTRL: return PICOFB_Key_RCTRL;
        case KEY_LEFTALT: return PICOFB_Key_LALT; 
        case KEY_RIGHTALT: return PICOFB_Key_RALT;
        case KEY_LEFTMETA: return PICOFB_Key_LSUPER; 
        case KEY_RIGHTMETA: return PICOFB_Key_RSUPER;
        case KEY_MINUS: return PICOFB_Key_MINUS; 
        case KEY_EQUAL: return PICOFB_Key_EQUALS;
        case KEY_LEFTBRACE: return PICOFB_Key_LBRACKET; 
        case KEY_RIGHTBRACE: return PICOFB_Key_RBRACKET;
        case KEY_BACKSLASH: return PICOFB_Key_BACKSLASH; 
        case KEY_SEMICOLON: return PICOFB_Key_SEMICOLON;
        case KEY_APOSTROPHE: return PICOFB_Key_APOSTROPHE; 
        case KEY_COMMA: return PICOFB_Key_COMMA;
        case KEY_DOT: return PICOFB_Key_PERIOD; 
        case KEY_SLASH: return PICOFB_Key_SLASH; 
        case KEY_GRAVE: return PICOFB_Key_GRAVE; 
        case KEY_CAPSLOCK: return PICOFB_Key_CAPSLOCK;
        default: return PICOFB_Key_UNKNOWN;
    }
}

static void PICOFB_WAYLAND_kb_key(void* d, struct wl_keyboard* kb, uint32_t srl, uint32_t t, uint32_t k, uint32_t s) {
    (void)kb; (void)srl; (void)t;
    PICOFB_Window* win = (PICOFB_Window*)d;
    PICOFB_Key key = PICOFB_WAYLAND_itt(k);
    if (key != PICOFB_Key_UNKNOWN) win->keyboard[key] = (s == WL_KEYBOARD_KEY_STATE_PRESSED);
}
static void PICOFB_WAYLAND_kb_keymap(void* d, struct wl_keyboard* k, uint32_t f, int32_t fd, uint32_t s) {(void)d;(void)k;(void)f;(void)s; close(fd);}
static void PICOFB_WAYLAND_kb_enter(void* d, struct wl_keyboard* k, uint32_t s, struct wl_surface* su, struct wl_array* ke) {(void)d;(void)k;(void)s;(void)su;(void)ke;}
static void PICOFB_WAYLAND_kb_leave(void* d, struct wl_keyboard* k, uint32_t s, struct wl_surface* su) {
    (void)k; (void)s; (void)su;
    PICOFB_Window* win = (PICOFB_Window*)d;
    memset(win->keyboard, 0, sizeof(win->keyboard));
}
static void PICOFB_WAYLAND_kb_mods(void* d, struct wl_keyboard* k, uint32_t s, uint32_t md, uint32_t ml, uint32_t mo, uint32_t g) {(void)d;(void)k;(void)s;(void)md;(void)ml;(void)mo;(void)g;}
static void PICOFB_WAYLAND_kb_repeat(void* d, struct wl_keyboard* k, int32_t r, int32_t de) {(void)d;(void)k;(void)r;(void)de;}
static const struct wl_keyboard_listener PICOFB_WAYLAND_kb_listener = {PICOFB_WAYLAND_kb_keymap, PICOFB_WAYLAND_kb_enter, PICOFB_WAYLAND_kb_leave, PICOFB_WAYLAND_kb_key, PICOFB_WAYLAND_kb_mods, PICOFB_WAYLAND_kb_repeat};
static void PICOFB_WAYLAND_ptr_enter(void* d, struct wl_pointer* p, uint32_t s, struct wl_surface* su, wl_fixed_t x, wl_fixed_t y) {(void)d;(void)p;(void)s;(void)su;(void)x;(void)y;}
static void PICOFB_WAYLAND_ptr_leave(void* d, struct wl_pointer* p, uint32_t s, struct wl_surface* su) {(void)d;(void)p;(void)s;(void)su;}
static void PICOFB_WAYLAND_ptr_motion(void* d, struct wl_pointer* p, uint32_t t, wl_fixed_t x, wl_fixed_t y) {
    (void)p; (void)t;
    PICOFB_Window* win = (PICOFB_Window*)d;
    int ix = wl_fixed_to_int(x), iy = wl_fixed_to_int(y);
    if (ix < 0) ix = 0; else if (ix > win->dont_touch.width) ix = win->dont_touch.width;
    if (iy < 0) iy = 0; else if (iy > win->dont_touch.height) iy = win->dont_touch.height;
    win->mouse.x = (uint16_t)ix; win->mouse.y = (uint16_t)iy;
}
static void PICOFB_WAYLAND_ptr_button(void* d, struct wl_pointer* p, uint32_t srl, uint32_t t, uint32_t b, uint32_t s) {
    (void)p; (void)srl; (void)t;
    PICOFB_Window* win = (PICOFB_Window*)d;
    bool pressed = (s == WL_POINTER_BUTTON_STATE_PRESSED);
    if (b== BTN_LEFT) win->mouse.left = pressed;
    else if (b == BTN_RIGHT) win->mouse.right = pressed;
    else if (b == BTN_MIDDLE) win->mouse.middle = pressed;
}
static void PICOFB_WAYLAND_ptr_axis(void* d, struct wl_pointer* p, uint32_t t, uint32_t a, wl_fixed_t v) {
    (void)p; (void)t;
    if (a != WL_POINTER_AXIS_VERTICAL_SCROLL) return;
    PICOFB_Window* win = (PICOFB_Window*)d;
    win->mouse.scroll_delta = (wl_fixed_to_double(v) > 0) ? -1 : (wl_fixed_to_double(v) < 0) ? 1 : 0;
}
static void PICOFB_WAYLAND_ptr_frame(void* d, struct wl_pointer* p) {(void)d;(void)p;}
static void PICOFB_WAYLAND_ptr_axis_source(void* d, struct wl_pointer* p, uint32_t s) {(void)d;(void)p;(void)s;}
static void PICOFB_WAYLAND_ptr_axis_stop(void* d, struct wl_pointer* p, uint32_t t, uint32_t a) {(void)d;(void)p;(void)t;(void)a;}
static void PICOFB_WAYLAND_ptr_axis_discrete(void* d, struct wl_pointer* p, uint32_t a, int32_t di) {(void)d;(void)p;(void)a;(void)di;}
static void PICOFB_WAYLAND_ptr_axis_value120(void* d, struct wl_pointer* p, uint32_t a, int32_t v) {(void)d;(void)p;(void)a;(void)v;}
static void PICOFB_WAYLAND_ptr_axis_relative_direction(void* d, struct wl_pointer* p, uint32_t a, uint32_t dir) {(void)d;(void)p;(void)a;(void)dir;}
static const struct wl_pointer_listener PICOFB_WAYLAND_ptr_listener = {PICOFB_WAYLAND_ptr_enter, PICOFB_WAYLAND_ptr_leave, PICOFB_WAYLAND_ptr_motion, PICOFB_WAYLAND_ptr_button, PICOFB_WAYLAND_ptr_axis, PICOFB_WAYLAND_ptr_frame, PICOFB_WAYLAND_ptr_axis_source, PICOFB_WAYLAND_ptr_axis_stop, PICOFB_WAYLAND_ptr_axis_discrete, PICOFB_WAYLAND_ptr_axis_value120, PICOFB_WAYLAND_ptr_axis_relative_direction};
static void PICOFB_WAYLAND_seat_caps(void* d, struct wl_seat* s, uint32_t c) {
    PICOFB_Window* win = (PICOFB_Window*)d;
    if (c & WL_SEAT_CAPABILITY_KEYBOARD) {
        win->dont_touch.kb = wl_seat_get_keyboard(s);
        wl_keyboard_add_listener(win->dont_touch.kb, &PICOFB_WAYLAND_kb_listener, win);
    }
    if (c & WL_SEAT_CAPABILITY_POINTER) {
        win->dont_touch.pointer = wl_seat_get_pointer(s);
        wl_pointer_add_listener(win->dont_touch.pointer, &PICOFB_WAYLAND_ptr_listener, win);
    }
}
static void PICOFB_WAYLAND_seat_name(void* d, struct wl_seat* s, const char* n) {(void)d;(void)s;(void)n;}
static const struct wl_seat_listener PICOFB_WAYLAND_seat_listener = {PICOFB_WAYLAND_seat_caps, PICOFB_WAYLAND_seat_name};
static void PICOFB_WAYLAND_xdg_surf_cfg(void* d, struct xdg_surface* xs, uint32_t srl) {
    PICOFB_Window* win = (PICOFB_Window*)d;
    xdg_surface_ack_configure(xs, srl);
    win->dont_touch.configured = true;
}
static const struct xdg_surface_listener PICOFB_WAYLAND_xdg_surf_listener = {PICOFB_WAYLAND_xdg_surf_cfg};
static void PICOFB_WAYLAND_xdg_top_cfg(void* d, struct xdg_toplevel* t, int32_t w, int32_t h, struct wl_array* s) {
    (void)t; (void)s;
    PICOFB_Window* win = (PICOFB_Window*)d;
    if (w > 0 && h > 0 && (w != win->dont_touch.width || h != win->dont_touch.height)) win->quit = true;
}
static void PICOFB_WAYLAND_xdg_top_close(void* d, struct xdg_toplevel* t) {(void)t; ((PICOFB_Window*)d)->quit = true;}
static void PICOFB_WAYLAND_xdg_top_cfg_bounds(void* d, struct xdg_toplevel* t, int32_t w, int32_t h) {(void)d;(void)t;(void)w;(void)h;}
static void PICOFB_WAYLAND_xdg_top_wm_capabilities(void* d, struct xdg_toplevel* t, struct wl_array* c) {(void)d;(void)t;(void)c;}
static const struct xdg_toplevel_listener PICOFB_WAYLAND_xdg_top_listener = {PICOFB_WAYLAND_xdg_top_cfg, PICOFB_WAYLAND_xdg_top_close, PICOFB_WAYLAND_xdg_top_cfg_bounds, PICOFB_WAYLAND_xdg_top_wm_capabilities};
static void PICOFB_WAYLAND_xdg_ping(void* d, struct xdg_wm_base* x, uint32_t srl) {(void)d; xdg_wm_base_pong(x, srl);}
static const struct xdg_wm_base_listener PICOFB_WAYLAND_xdg_listener = {PICOFB_WAYLAND_xdg_ping};
static void PICOFB_WAYLAND_reg_global(void* d, struct wl_registry* r, uint32_t n, const char* i, uint32_t v) {
    (void)v;
    PICOFB_Window* win = (PICOFB_Window*)d;
    if (strcmp(i, wl_compositor_interface.name) == 0) win->dont_touch.compositor = wl_registry_bind(r, n, &wl_compositor_interface, 4);
    else if (strcmp(i, wl_shm_interface.name) == 0) win->dont_touch.shm = wl_registry_bind(r, n, &wl_shm_interface, 1);
    else if (strcmp(i, xdg_wm_base_interface.name) == 0) {
        win->dont_touch.xdg_wm_base = wl_registry_bind(r, n, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(win->dont_touch.xdg_wm_base, &PICOFB_WAYLAND_xdg_listener, win);
    }
    else if (strcmp(i, wl_seat_interface.name) == 0) {
        win->dont_touch.seat = wl_registry_bind(r, n, &wl_seat_interface, 5);
        wl_seat_add_listener(win->dont_touch.seat, &PICOFB_WAYLAND_seat_listener, win);
    }
}
static void PICOFB_WAYLAND_reg_remove(void* d, struct wl_registry* r, uint32_t n) {(void)d;(void)r;(void)n;}
static const struct wl_registry_listener PICOFB_WAYLAND_reg_listener = {PICOFB_WAYLAND_reg_global, PICOFB_WAYLAND_reg_remove};
#ifdef PICOFB_WAYLAND_OLD_MEMFD
static inline int PICOFB_WAYLAND_memfd_create(const char* n, unsigned int f) {
    return syscall(SYS_memfd_create, n, f);
}
#else
static inline int PICOFB_WAYLAND_memfd_create(const char* n, unsigned int f) {
    return memfd_create(n, f);
}
#endif

PICOFB_DEF bool PICOFB_init(const char* window_title, uint16_t width, uint16_t height, PICOFB_Window* picofb_window) {
    if (!picofb_window) return false;
    memset(picofb_window, 0, sizeof(PICOFB_Window));
    picofb_window->dont_touch.shm_fd = -1;
    picofb_window->dont_touch.width = width; picofb_window->dont_touch.height = height;
    picofb_window->dont_touch.display = wl_display_connect(NULL);
    if (!picofb_window->dont_touch.display) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.registry = wl_display_get_registry(picofb_window->dont_touch.display);
    wl_registry_add_listener(picofb_window->dont_touch.registry, &PICOFB_WAYLAND_reg_listener, picofb_window);
    wl_display_roundtrip(picofb_window->dont_touch.display);
    if (!picofb_window->dont_touch.compositor || !picofb_window->dont_touch.shm || !picofb_window->dont_touch.xdg_wm_base) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.surface = wl_compositor_create_surface(picofb_window->dont_touch.compositor);
    picofb_window->dont_touch.xdg_surface = xdg_wm_base_get_xdg_surface(picofb_window->dont_touch.xdg_wm_base, picofb_window->dont_touch.surface);
    xdg_surface_add_listener(picofb_window->dont_touch.xdg_surface, &PICOFB_WAYLAND_xdg_surf_listener, picofb_window);
    picofb_window->dont_touch.xdg_toplevel = xdg_surface_get_toplevel(picofb_window->dont_touch.xdg_surface);
    xdg_toplevel_add_listener(picofb_window->dont_touch.xdg_toplevel, &PICOFB_WAYLAND_xdg_top_listener, picofb_window);
    xdg_toplevel_set_title(picofb_window->dont_touch.xdg_toplevel, window_title ? window_title : "");
    xdg_toplevel_set_min_size(picofb_window->dont_touch.xdg_toplevel, width, height);
    xdg_toplevel_set_max_size(picofb_window->dont_touch.xdg_toplevel, width, height);
    size_t size = width * height * 4;
    picofb_window->dont_touch.shm_fd = PICOFB_WAYLAND_memfd_create(window_title ? window_title : "", MFD_CLOEXEC);
    if (picofb_window->dont_touch.shm_fd < 0 || ftruncate(picofb_window->dont_touch.shm_fd, size) < 0) {PICOFB_cleanup(picofb_window); return false;}
    void* data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, picofb_window->dont_touch.shm_fd, 0);
    if (data == MAP_FAILED) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->frame_buffer = (uint32_t*)data;
    struct wl_shm_pool* pool = wl_shm_create_pool(picofb_window->dont_touch.shm, picofb_window->dont_touch.shm_fd, size);
    picofb_window->dont_touch.buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    wl_surface_commit(picofb_window->dont_touch.surface);
    while (!picofb_window->dont_touch.configured && !picofb_window->quit) wl_display_dispatch(picofb_window->dont_touch.display);
    return true;
}

PICOFB_DEF void PICOFB_update(PICOFB_Window* picofb_window) {
    picofb_window->mouse.scroll_delta = 0;
    wl_display_flush(picofb_window->dont_touch.display);
    struct pollfd pfd = {wl_display_get_fd(picofb_window->dont_touch.display), POLLIN, 0};
    int dispatch_result = (poll(&pfd, 1, 0) > 0) ? wl_display_dispatch(picofb_window->dont_touch.display) : wl_display_dispatch_pending(picofb_window->dont_touch.display);
    if (dispatch_result < 0) { picofb_window->quit = true; return; }
    else wl_display_dispatch_pending(picofb_window->dont_touch.display);
    wl_surface_attach(picofb_window->dont_touch.surface, picofb_window->dont_touch.buffer, 0, 0);
    wl_surface_damage(picofb_window->dont_touch.surface, 0, 0, picofb_window->dont_touch.width, picofb_window->dont_touch.height);
    wl_surface_commit(picofb_window->dont_touch.surface);
    if (wl_display_roundtrip(picofb_window->dont_touch.display) < 0) { picofb_window->quit = true; return; }
}

PICOFB_DEF void PICOFB_cleanup(PICOFB_Window* picofb_window) {
    if (picofb_window->frame_buffer) munmap(picofb_window->frame_buffer, picofb_window->dont_touch.width * picofb_window->dont_touch.height * 4);
    if (picofb_window->dont_touch.buffer) wl_buffer_destroy(picofb_window->dont_touch.buffer);
    if (picofb_window->dont_touch.shm_fd >= 0) close(picofb_window->dont_touch.shm_fd);
    if (picofb_window->dont_touch.kb) wl_keyboard_destroy(picofb_window->dont_touch.kb);
    if (picofb_window->dont_touch.pointer) wl_pointer_destroy(picofb_window->dont_touch.pointer);
    if (picofb_window->dont_touch.seat) wl_seat_destroy(picofb_window->dont_touch.seat);
    if (picofb_window->dont_touch.xdg_toplevel) xdg_toplevel_destroy(picofb_window->dont_touch.xdg_toplevel);
    if (picofb_window->dont_touch.xdg_surface) xdg_surface_destroy(picofb_window->dont_touch.xdg_surface);
    if (picofb_window->dont_touch.surface) wl_surface_destroy(picofb_window->dont_touch.surface);
    if (picofb_window->dont_touch.xdg_wm_base) xdg_wm_base_destroy(picofb_window->dont_touch.xdg_wm_base);
    if (picofb_window->dont_touch.shm) wl_shm_destroy(picofb_window->dont_touch.shm);
    if (picofb_window->dont_touch.compositor) wl_compositor_destroy(picofb_window->dont_touch.compositor);
    if (picofb_window->dont_touch.registry) wl_registry_destroy(picofb_window->dont_touch.registry);
    if (picofb_window->dont_touch.display) wl_display_disconnect(picofb_window->dont_touch.display);
    memset(picofb_window, 0, sizeof(PICOFB_Window));
}

#endif // PICOFB_WAYLAND_BACKEND

//================================================================================================================================

#ifdef PICOFB_WIN32_BACKEND

static inline PICOFB_Key PICOFB_WIN32_itt(WPARAM vk, LPARAM lparam) {
    bool extended = (lparam & 0x01000000) != 0;
    switch(vk) {
        case 'A': return PICOFB_Key_a;
        case 'B': return PICOFB_Key_b;
        case 'C': return PICOFB_Key_c;
        case 'D': return PICOFB_Key_d;
        case 'E': return PICOFB_Key_e;
        case 'F': return PICOFB_Key_f;
        case 'G': return PICOFB_Key_g;
        case 'H': return PICOFB_Key_h;
        case 'I': return PICOFB_Key_i;
        case 'J': return PICOFB_Key_j;
        case 'K': return PICOFB_Key_k;
        case 'L': return PICOFB_Key_l;
        case 'M': return PICOFB_Key_m;
        case 'N': return PICOFB_Key_n;
        case 'O': return PICOFB_Key_o;
        case 'P': return PICOFB_Key_p;
        case 'Q': return PICOFB_Key_q;
        case 'R': return PICOFB_Key_r;
        case 'S': return PICOFB_Key_s;
        case 'T': return PICOFB_Key_t;
        case 'U': return PICOFB_Key_u;
        case 'V': return PICOFB_Key_v;
        case 'W': return PICOFB_Key_w;
        case 'X': return PICOFB_Key_x;
        case 'Y': return PICOFB_Key_y;
        case 'Z': return PICOFB_Key_z;
        case '0': return PICOFB_Key_0;
        case '1': return PICOFB_Key_1;
        case '2': return PICOFB_Key_2;
        case '3': return PICOFB_Key_3;
        case '4': return PICOFB_Key_4;
        case '5': return PICOFB_Key_5;
        case '6': return PICOFB_Key_6;
        case '7': return PICOFB_Key_7;
        case '8': return PICOFB_Key_8;
        case '9': return PICOFB_Key_9;
        case VK_F1: return PICOFB_Key_F1;
        case VK_F2: return PICOFB_Key_F2;
        case VK_F3: return PICOFB_Key_F3;
        case VK_F4: return PICOFB_Key_F4;
        case VK_F5: return PICOFB_Key_F5;
        case VK_F6: return PICOFB_Key_F6;
        case VK_F7: return PICOFB_Key_F7;
        case VK_F8: return PICOFB_Key_F8;
        case VK_F9: return PICOFB_Key_F9;
        case VK_F10: return PICOFB_Key_F10;
        case VK_F11: return PICOFB_Key_F11;
        case VK_F12: return PICOFB_Key_F12;
        case VK_UP: return PICOFB_Key_UP;
        case VK_DOWN: return PICOFB_Key_DOWN;
        case VK_LEFT: return PICOFB_Key_LEFT;
        case VK_RIGHT: return PICOFB_Key_RIGHT;
        case VK_ESCAPE: return PICOFB_Key_ESC;
        case VK_SPACE: return PICOFB_Key_SPACE;
        case VK_RETURN: return PICOFB_Key_ENTER;
        case VK_TAB: return PICOFB_Key_TAB;
        case VK_BACK: return PICOFB_Key_BACKSPACE;
        case VK_DELETE: return PICOFB_Key_DELETE;
        case VK_INSERT: return PICOFB_Key_INSERT;
        case VK_HOME: return PICOFB_Key_HOME;
        case VK_END: return PICOFB_Key_END;
        case VK_PRIOR: return PICOFB_Key_PAGEUP;
        case VK_NEXT: return PICOFB_Key_PAGEDOWN;
        case VK_SHIFT: return (((lparam >> 16) & 0xFF) == 0x36) ? PICOFB_Key_RSHIFT : PICOFB_Key_LSHIFT;
        case VK_CONTROL: return extended ? PICOFB_Key_RCTRL : PICOFB_Key_LCTRL;
        case VK_MENU: return extended ? PICOFB_Key_RALT : PICOFB_Key_LALT;
        case VK_LWIN: return PICOFB_Key_LSUPER;
        case VK_RWIN: return PICOFB_Key_RSUPER;
        case VK_OEM_MINUS: return PICOFB_Key_MINUS;
        case VK_OEM_PLUS: return PICOFB_Key_EQUALS;
        case VK_OEM_4: return PICOFB_Key_LBRACKET;
        case VK_OEM_6: return PICOFB_Key_RBRACKET;
        case VK_OEM_5: return PICOFB_Key_BACKSLASH;
        case VK_OEM_1: return PICOFB_Key_SEMICOLON;
        case VK_OEM_7: return PICOFB_Key_APOSTROPHE;
        case VK_OEM_COMMA: return PICOFB_Key_COMMA;
        case VK_OEM_PERIOD: return PICOFB_Key_PERIOD;
        case VK_OEM_2: return PICOFB_Key_SLASH;
        case VK_OEM_3: return PICOFB_Key_GRAVE;
        case VK_CAPITAL: return PICOFB_Key_CAPSLOCK;
        default: return PICOFB_Key_UNKNOWN;
    }
}

static LRESULT CALLBACK PICOFB_WIN32_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    PICOFB_Window* window = (PICOFB_Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            PICOFB_Key key = PICOFB_WIN32_itt(wparam, lparam);
            if (key != PICOFB_Key_UNKNOWN && !(lparam & 0x40000000)) window->keyboard[key] = true;
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            PICOFB_Key key = PICOFB_WIN32_itt(wparam, lparam);
            if (key != PICOFB_Key_UNKNOWN) window->keyboard[key] = false;
            return 0;
        }
        case WM_KILLFOCUS:
            memset(window->keyboard, 0, sizeof(window->keyboard));
            return 0;
        case WM_LBUTTONDOWN: window->mouse.left = true; return 0;
        case WM_LBUTTONUP: window->mouse.left = false; return 0;
        case WM_RBUTTONDOWN: window->mouse.right = true; return 0;
        case WM_RBUTTONUP: window->mouse.right = false; return 0;
        case WM_MBUTTONDOWN: window->mouse.middle = true; return 0;
        case WM_MBUTTONUP: window->mouse.middle = false; return 0;
        case WM_MOUSEMOVE:
            window->mouse.x = LOWORD(lparam);
            window->mouse.y = HIWORD(lparam);
            return 0;
        case WM_MOUSEWHEEL:
            window->mouse.scroll_delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
            return 0;
        case WM_ERASEBKGND: return 1;
        case WM_CLOSE: if (window) window->quit = true; return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
        default: return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

PICOFB_DEF bool PICOFB_init(const char* window_title, uint16_t width, uint16_t height, PICOFB_Window* picofb_window) {
    if (!picofb_window) return false;
    memset(picofb_window, 0, sizeof(PICOFB_Window));
    picofb_window->dont_touch.width = width; picofb_window->dont_touch.height = height; 
    snprintf(picofb_window->dont_touch.class_name_storage, sizeof(picofb_window->dont_touch.class_name_storage), "PICOFB_%p", (void*)picofb_window);
    picofb_window->dont_touch.wc.cbSize = sizeof(WNDCLASSEX);
    picofb_window->dont_touch.wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    picofb_window->dont_touch.wc.lpfnWndProc = PICOFB_WIN32_window_proc;
    picofb_window->dont_touch.wc.cbClsExtra = 0;
    picofb_window->dont_touch.wc.cbWndExtra = 0;
    picofb_window->dont_touch.wc.hInstance = GetModuleHandle(NULL);
    picofb_window->dont_touch.wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    picofb_window->dont_touch.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    picofb_window->dont_touch.wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    picofb_window->dont_touch.wc.lpszMenuName = NULL;
    picofb_window->dont_touch.wc.lpszClassName = picofb_window->dont_touch.class_name_storage;
    picofb_window->dont_touch.wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassEx(&picofb_window->dont_touch.wc)) {PICOFB_cleanup(picofb_window); return false;}
    DWORD style = WS_POPUP;
    DWORD ex_style = WS_EX_TOOLWINDOW;
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	POINT origin = { 0, 0 };
	HMONITOR monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monitor_info = {};
	monitor_info.cbSize = sizeof(MONITORINFO);
	int x = 0;
	int y = 0;
	if (GetMonitorInfo(monitor, &monitor_info)) {
		int monitor_width = monitor_info.rcMonitor.right - monitor_info.rcMonitor.left;
		int monitor_height = monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top;
		x = monitor_info.rcMonitor.left + (monitor_width - width) / 2;
		y = monitor_info.rcMonitor.top + (monitor_height - height) / 2;
	}
	picofb_window->dont_touch.hwnd = CreateWindowEx(ex_style, picofb_window->dont_touch.class_name_storage, window_title ? window_title : "", style, x, y, width, height, NULL, NULL, GetModuleHandle(NULL), NULL);
    if (!picofb_window->dont_touch.hwnd) {UnregisterClass(picofb_window->dont_touch.class_name_storage, GetModuleHandle(NULL)); PICOFB_cleanup(picofb_window); return false;}
	SetWindowPos(picofb_window->dont_touch.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowLongPtr(picofb_window->dont_touch.hwnd, GWLP_USERDATA, (LONG_PTR)picofb_window);
    picofb_window->dont_touch.hdc = GetDC(picofb_window->dont_touch.hwnd);
    if (!picofb_window->dont_touch.hdc) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.hdc_mem = CreateCompatibleDC(picofb_window->dont_touch.hdc);
    if (!picofb_window->dont_touch.hdc_mem) {PICOFB_cleanup(picofb_window); return false;}
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    picofb_window->dont_touch.hbitmap = CreateDIBSection(picofb_window->dont_touch.hdc_mem, &bmi, DIB_RGB_COLORS, (void**)&picofb_window->frame_buffer, NULL, 0);
    if (!picofb_window->dont_touch.hbitmap) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.hbitmap_old = (HBITMAP)SelectObject(picofb_window->dont_touch.hdc_mem, picofb_window->dont_touch.hbitmap);
    return true;
}

PICOFB_DEF void PICOFB_update(PICOFB_Window* picofb_window) {
    picofb_window->mouse.scroll_delta = 0;
    while (PeekMessage(&picofb_window->dont_touch.msg, picofb_window->dont_touch.hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&picofb_window->dont_touch.msg);
        DispatchMessage(&picofb_window->dont_touch.msg);
    }
    BitBlt(picofb_window->dont_touch.hdc, 0, 0, picofb_window->dont_touch.width, picofb_window->dont_touch.height, picofb_window->dont_touch.hdc_mem, 0, 0, SRCCOPY);
}

PICOFB_DEF void PICOFB_cleanup(PICOFB_Window* picofb_window) {
    if (picofb_window->dont_touch.hbitmap) {
        if (picofb_window->dont_touch.hdc_mem && picofb_window->dont_touch.hbitmap_old) SelectObject(picofb_window->dont_touch.hdc_mem, picofb_window->dont_touch.hbitmap_old);
        DeleteObject(picofb_window->dont_touch.hbitmap);
    }
    if (picofb_window->dont_touch.hdc_mem) DeleteDC(picofb_window->dont_touch.hdc_mem);
    if (picofb_window->dont_touch.hdc && picofb_window->dont_touch.hwnd) ReleaseDC(picofb_window->dont_touch.hwnd, picofb_window->dont_touch.hdc);
    if (picofb_window->dont_touch.hwnd) DestroyWindow(picofb_window->dont_touch.hwnd);
    if (picofb_window->dont_touch.class_name_storage[0]) UnregisterClass(picofb_window->dont_touch.class_name_storage, GetModuleHandle(NULL));
    memset(picofb_window, 0, sizeof(PICOFB_Window));
}

#endif // PICOFB_WIN32_BACKEND

//================================================================================================================================

#ifdef PICOFB_SDL_BACKEND

static inline PICOFB_Key PICOFB_SDL_itt(SDL_Scancode sc) {
    switch(sc) {
        case SDL_SCANCODE_A: return PICOFB_Key_a;
        case SDL_SCANCODE_B: return PICOFB_Key_b;
        case SDL_SCANCODE_C: return PICOFB_Key_c;
        case SDL_SCANCODE_D: return PICOFB_Key_d;
        case SDL_SCANCODE_E: return PICOFB_Key_e;
        case SDL_SCANCODE_F: return PICOFB_Key_f;
        case SDL_SCANCODE_G: return PICOFB_Key_g;
        case SDL_SCANCODE_H: return PICOFB_Key_h;
        case SDL_SCANCODE_I: return PICOFB_Key_i;
        case SDL_SCANCODE_J: return PICOFB_Key_j;
        case SDL_SCANCODE_K: return PICOFB_Key_k;
        case SDL_SCANCODE_L: return PICOFB_Key_l;
        case SDL_SCANCODE_M: return PICOFB_Key_m;
        case SDL_SCANCODE_N: return PICOFB_Key_n;
        case SDL_SCANCODE_O: return PICOFB_Key_o;
        case SDL_SCANCODE_P: return PICOFB_Key_p;
        case SDL_SCANCODE_Q: return PICOFB_Key_q;
        case SDL_SCANCODE_R: return PICOFB_Key_r;
        case SDL_SCANCODE_S: return PICOFB_Key_s;
        case SDL_SCANCODE_T: return PICOFB_Key_t;
        case SDL_SCANCODE_U: return PICOFB_Key_u;
        case SDL_SCANCODE_V: return PICOFB_Key_v;
        case SDL_SCANCODE_W: return PICOFB_Key_w;
        case SDL_SCANCODE_X: return PICOFB_Key_x;
        case SDL_SCANCODE_Y: return PICOFB_Key_y;
        case SDL_SCANCODE_Z: return PICOFB_Key_z;
        case SDL_SCANCODE_0: return PICOFB_Key_0;
        case SDL_SCANCODE_1: return PICOFB_Key_1;
        case SDL_SCANCODE_2: return PICOFB_Key_2;
        case SDL_SCANCODE_3: return PICOFB_Key_3;
        case SDL_SCANCODE_4: return PICOFB_Key_4;
        case SDL_SCANCODE_5: return PICOFB_Key_5;
        case SDL_SCANCODE_6: return PICOFB_Key_6;
        case SDL_SCANCODE_7: return PICOFB_Key_7;
        case SDL_SCANCODE_8: return PICOFB_Key_8;
        case SDL_SCANCODE_9: return PICOFB_Key_9;
        case SDL_SCANCODE_F1: return PICOFB_Key_F1;
        case SDL_SCANCODE_F2: return PICOFB_Key_F2;
        case SDL_SCANCODE_F3: return PICOFB_Key_F3;
        case SDL_SCANCODE_F4: return PICOFB_Key_F4;
        case SDL_SCANCODE_F5: return PICOFB_Key_F5;
        case SDL_SCANCODE_F6: return PICOFB_Key_F6;
        case SDL_SCANCODE_F7: return PICOFB_Key_F7;
        case SDL_SCANCODE_F8: return PICOFB_Key_F8;
        case SDL_SCANCODE_F9: return PICOFB_Key_F9;
        case SDL_SCANCODE_F10: return PICOFB_Key_F10;
        case SDL_SCANCODE_F11: return PICOFB_Key_F11;
        case SDL_SCANCODE_F12: return PICOFB_Key_F12;
        case SDL_SCANCODE_UP: return PICOFB_Key_UP;
        case SDL_SCANCODE_DOWN: return PICOFB_Key_DOWN;
        case SDL_SCANCODE_LEFT: return PICOFB_Key_LEFT;
        case SDL_SCANCODE_RIGHT: return PICOFB_Key_RIGHT;
        case SDL_SCANCODE_ESCAPE: return PICOFB_Key_ESC;
        case SDL_SCANCODE_SPACE: return PICOFB_Key_SPACE;
        case SDL_SCANCODE_RETURN: return PICOFB_Key_ENTER;
        case SDL_SCANCODE_TAB: return PICOFB_Key_TAB;
        case SDL_SCANCODE_BACKSPACE: return PICOFB_Key_BACKSPACE;
        case SDL_SCANCODE_DELETE: return PICOFB_Key_DELETE;
        case SDL_SCANCODE_INSERT: return PICOFB_Key_INSERT;
        case SDL_SCANCODE_HOME: return PICOFB_Key_HOME;
        case SDL_SCANCODE_END: return PICOFB_Key_END;
        case SDL_SCANCODE_PAGEUP: return PICOFB_Key_PAGEUP;
        case SDL_SCANCODE_PAGEDOWN: return PICOFB_Key_PAGEDOWN;
        case SDL_SCANCODE_LSHIFT: return PICOFB_Key_LSHIFT;
        case SDL_SCANCODE_RSHIFT: return PICOFB_Key_RSHIFT;
        case SDL_SCANCODE_LCTRL: return PICOFB_Key_LCTRL;
        case SDL_SCANCODE_RCTRL: return PICOFB_Key_RCTRL;
        case SDL_SCANCODE_LALT: return PICOFB_Key_LALT;
        case SDL_SCANCODE_RALT: return PICOFB_Key_RALT;
        case SDL_SCANCODE_LGUI: return PICOFB_Key_LSUPER;
        case SDL_SCANCODE_RGUI: return PICOFB_Key_RSUPER;
        case SDL_SCANCODE_MINUS: return PICOFB_Key_MINUS;
        case SDL_SCANCODE_EQUALS: return PICOFB_Key_EQUALS;
        case SDL_SCANCODE_LEFTBRACKET: return PICOFB_Key_LBRACKET;
        case SDL_SCANCODE_RIGHTBRACKET: return PICOFB_Key_RBRACKET;
        case SDL_SCANCODE_BACKSLASH: return PICOFB_Key_BACKSLASH;
        case SDL_SCANCODE_SEMICOLON: return PICOFB_Key_SEMICOLON;
        case SDL_SCANCODE_APOSTROPHE: return PICOFB_Key_APOSTROPHE;
        case SDL_SCANCODE_COMMA: return PICOFB_Key_COMMA;
        case SDL_SCANCODE_PERIOD: return PICOFB_Key_PERIOD;
        case SDL_SCANCODE_SLASH: return PICOFB_Key_SLASH;
        case SDL_SCANCODE_GRAVE: return PICOFB_Key_GRAVE;
        case SDL_SCANCODE_CAPSLOCK: return PICOFB_Key_CAPSLOCK;
        default: return PICOFB_Key_UNKNOWN;
    }
}

PICOFB_DEF bool PICOFB_init(const char* window_title, uint16_t width, uint16_t height, PICOFB_Window* picofb_window) {
    if (!picofb_window) return false;
    memset(picofb_window, 0, sizeof(PICOFB_Window));
    picofb_window->frame_buffer = calloc(width * height, sizeof(uint32_t));
    if (!picofb_window->frame_buffer) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.width = width; picofb_window->dont_touch.height = height;
    if (!SDL_Init(SDL_INIT_VIDEO)) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.window = SDL_CreateWindow(window_title ? window_title : "", width, height, 0);
    if (!picofb_window->dont_touch.window) {PICOFB_cleanup(picofb_window); return false;}
    SDL_SetWindowPosition(picofb_window->dont_touch.window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    picofb_window->dont_touch.renderer = SDL_CreateRenderer(picofb_window->dont_touch.window, NULL);
    if (!picofb_window->dont_touch.renderer) {PICOFB_cleanup(picofb_window); return false;}
    picofb_window->dont_touch.texture = SDL_CreateTexture(picofb_window->dont_touch.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!picofb_window->dont_touch.texture) {PICOFB_cleanup(picofb_window); return false;}
    SDL_SetTextureBlendMode(picofb_window->dont_touch.texture, SDL_BLENDMODE_NONE);
    return true;
}

PICOFB_DEF void PICOFB_update(PICOFB_Window* picofb_window) {
    picofb_window->mouse.scroll_delta = 0;
    SDL_UpdateTexture(picofb_window->dont_touch.texture, NULL, picofb_window->frame_buffer, picofb_window->dont_touch.width * sizeof(uint32_t));
    SDL_RenderTexture(picofb_window->dont_touch.renderer, picofb_window->dont_touch.texture, NULL, NULL);
    SDL_RenderPresent(picofb_window->dont_touch.renderer);
    while (SDL_PollEvent(&picofb_window->dont_touch.event)) {
        switch(picofb_window->dont_touch.event.type) {
            case SDL_EVENT_KEY_DOWN: {
                PICOFB_Key key = PICOFB_SDL_itt(picofb_window->dont_touch.event.key.scancode);
                if (key != PICOFB_Key_UNKNOWN) picofb_window->keyboard[key] = true;
                break;
            }
            case SDL_EVENT_KEY_UP: {
                PICOFB_Key key = PICOFB_SDL_itt(picofb_window->dont_touch.event.key.scancode);
                if (key != PICOFB_Key_UNKNOWN) picofb_window->keyboard[key] = false;
                break;
            }
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                memset(picofb_window->keyboard, 0, sizeof(picofb_window->keyboard));
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                picofb_window->quit = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (picofb_window->dont_touch.event.button.button == SDL_BUTTON_LEFT) picofb_window->mouse.left = true;
                if (picofb_window->dont_touch.event.button.button == SDL_BUTTON_RIGHT) picofb_window->mouse.right = true;
                if (picofb_window->dont_touch.event.button.button == SDL_BUTTON_MIDDLE) picofb_window->mouse.middle = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (picofb_window->dont_touch.event.button.button == SDL_BUTTON_LEFT) picofb_window->mouse.left = false;
                if (picofb_window->dont_touch.event.button.button == SDL_BUTTON_RIGHT) picofb_window->mouse.right = false;
                if (picofb_window->dont_touch.event.button.button == SDL_BUTTON_MIDDLE) picofb_window->mouse.middle = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                picofb_window->mouse.x = (uint16_t)picofb_window->dont_touch.event.motion.x;
                picofb_window->mouse.y = (uint16_t)picofb_window->dont_touch.event.motion.y;
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                picofb_window->mouse.scroll_delta = (int8_t)picofb_window->dont_touch.event.wheel.y;
                break;
            case SDL_EVENT_QUIT:
                picofb_window->quit = true;
                break;
        }
    }
}

PICOFB_DEF void PICOFB_cleanup(PICOFB_Window* picofb_window) {
    if (picofb_window->dont_touch.texture) SDL_DestroyTexture(picofb_window->dont_touch.texture);
    if (picofb_window->dont_touch.renderer) SDL_DestroyRenderer(picofb_window->dont_touch.renderer);
    if (picofb_window->dont_touch.window) SDL_DestroyWindow(picofb_window->dont_touch.window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    if (picofb_window->frame_buffer) free(picofb_window->frame_buffer);
    memset(picofb_window, 0, sizeof(PICOFB_Window));
}

#endif // PICOFB_SDL_BACKEND

//================================================================================================================================

#endif // PICOFB_IMPLEMENTATION

//================================================================================================================================
// UTIL FUNCTIONS
//================================================================================================================================

static inline uint32_t PICOFB_color_argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;}
static inline void PICOFB_set_pixel(PICOFB_Window* picofb_window, uint16_t x, uint16_t y, uint32_t color){
    picofb_window->frame_buffer[y*picofb_window->dont_touch.width+x] = color;
}
static inline void PICOFB_clear(PICOFB_Window* picofb_window){
    memset(picofb_window->frame_buffer, 0, picofb_window->dont_touch.width * picofb_window->dont_touch.height * sizeof(uint32_t));
}

static inline bool PICOFB_key_pressed(PICOFB_Window* picofb_window, PICOFB_Key key) {
    return picofb_window->keyboard[key];
}

static inline void PICOFB_save_ppm(PICOFB_Window* picofb_window, const char *path){
    unsigned char* row = (unsigned char*)malloc((picofb_window->dont_touch.width * 3) * sizeof(unsigned char));
    if (!row) return;
    FILE *f = fopen(path, "wb");
    if (!f) {free(row); return;}
    fprintf(f, "P6\n%d %d\n255\n", picofb_window->dont_touch.width, picofb_window->dont_touch.height);
    for (size_t y = 0; y < picofb_window->dont_touch.height; ++y){
        for (size_t x = 0; x < picofb_window->dont_touch.width; ++x){
            uint32_t px = picofb_window->frame_buffer[y * picofb_window->dont_touch.width + x];
            row[x * 3 + 0] = (px >> 16) & 0xFF;
            row[x * 3 + 1] = (px >> 8) & 0xFF;
            row[x * 3 + 2] =  px & 0xFF;
        }
        fwrite(row, 1, picofb_window->dont_touch.width * 3, f);
    }
    fclose(f);
    free(row);
}

//================================================================================================================================

#endif // PICOFB_H_

/*
LICENSE:
    MIT License

    Copyright (c) 2025 Arin Upadhyay

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

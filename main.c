
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "err_macros.c"
#include "match.h"
#include "inttypes.h"

GPU_Target *screen;


#define SDL_INIT_FLAGS SDL_INIT_AUDIO | SDL_INIT_VIDEO
#define ERRBUF_LEN 1024

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define FPS 60

// #FUNCS

void on_input(SDL_Event *e);

void on_key_pressed(SDL_Keycode key);

void on_key_released(SDL_Keycode key);

bool is_key_pressed(SDL_Scancode scancode);

void render(double delta);

void tick(double delta);

int main() {

    char errbuf[ERRBUF_LEN] = {0};

    if (SDL_Init(SDL_INIT_FLAGS)) err_exit(
        "SDL Couldn't init! error: '%s'", SDL_GetErrorMsg(errbuf, ERRBUF_LEN)
    );

    screen = GPU_Init(WINDOW_WIDTH, WINDOW_HEIGHT, GPU_DEFAULT_INIT_FLAGS);
    if (!screen) err_exit(
        "GPU Couldn't init! error: '%s'", GPU_GetErrorString(GPU_PopErrorCode().error)
    );

    u64 time = SDL_GetTicks64();
    double frame_timer = 0;

    while (true) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            on_input(&e);
        }
        
        u64 delta_mili = SDL_GetTicks64() - time;
        time = SDL_GetTicks64();

        double delta = (double)delta_mili / 1000;
        frame_timer += delta;

        if (frame_timer > 1.0/FPS) {
            tick(delta);
            render(delta);
        }
        
    }


    getchar();
}

void on_input(SDL_Event *e) {
    match(e->type) {
        case (SDL_QUIT) {
            exit(EXIT_SUCCESS);
        }
        case (SDL_KEYDOWN) {
            on_key_pressed(e->key.keysym.sym);
        }
        case (SDL_KEYUP) {
            on_key_released(e->key.keysym.sym);
        }
    }
}


void on_key_pressed(SDL_Keycode key) {
    match (key) {

    }
}

void on_key_released(SDL_Keycode key) {
    match (key) {
        
    }
}

bool is_key_pressed(SDL_Scancode scancode) {

    assert(scancode >= 0 && scancode < SDL_NUM_SCANCODES);

    return SDL_GetKeyboardState(NULL)[scancode];
}

void tick(double delta) {
    
}

void render(double delta) {

}


// #END
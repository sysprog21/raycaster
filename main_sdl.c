#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "raycaster.h"
#include "raycaster_fixed.h"
#include "raycaster_float.h"
#include "renderer.h"

static void draw_buffer(SDL_Renderer *sdlRenderer,
                        SDL_Texture *sdlTexture,
                        uint32_t *fb,
                        int dx)
{
    int pitch = 0;
    void *pixelsPtr;
    if (SDL_LockTexture(sdlTexture, NULL, &pixelsPtr, &pitch)) {
        fprintf(stderr, "Unable to lock texture");
        exit(1);
    }
    memcpy(pixelsPtr, fb, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    SDL_UnlockTexture(sdlTexture);
    SDL_Rect r;
    r.x = dx * SCREEN_SCALE;
    r.y = 0;
    r.w = SCREEN_WIDTH * SCREEN_SCALE;
    r.h = SCREEN_HEIGHT * SCREEN_SCALE;
    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &r);
}

static bool process_event(const SDL_Event *event,
                          int *moveDirection,
                          int *rotateDirection)
{
    if (event->type == SDL_QUIT) {
        return true;
    } else if ((event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) &&
               event->key.repeat == 0) {
        SDL_KeyboardEvent k = event->key;
        bool p = event->type == SDL_KEYDOWN;
        switch (k.keysym.sym) {
        case SDLK_ESCAPE:
            return true;
            break;
        case SDLK_UP:
            *moveDirection = p ? 1 : 0;
            break;
        case SDLK_DOWN:
            *moveDirection = p ? -1 : 0;
            break;
        case SDLK_LEFT:
            *rotateDirection = p ? -1 : 0;
            break;
        case SDLK_RIGHT:
            *rotateDirection = p ? 1 : 0;
            break;
        default:
            break;
        }
    }
    return false;
}
int main(int argc, char *args[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        SDL_Window *sdlWindow =
            SDL_CreateWindow("RayCaster [fixed-point vs. floating-point]",
                             SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             SCREEN_SCALE * (SCREEN_WIDTH * 2 + 1),
                             SCREEN_SCALE * SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if (sdlWindow == NULL) {
            printf("Window could not be created! SDL_Error: %s\n",
                   SDL_GetError());
        } else {
            Game game;
            RayCaster *floatCaster = RayCasterFloatConstruct();
            Renderer floatRenderer = RendererConstruct(floatCaster);
            uint32_t floatBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
            RayCaster *fixedCaster = RayCasterFixedConstruct();
            Renderer fixedRenderer = RendererConstruct(fixedCaster);
            uint32_t fixedBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
            int moveDirection = 0;
            int rotateDirection = 0;
            bool isExiting = false;
            const Uint64 tickFrequency = SDL_GetPerformanceFrequency();
            Uint64 tickCounter = SDL_GetPerformanceCounter();
            SDL_Event event;

            SDL_Renderer *sdlRenderer = SDL_CreateRenderer(
                sdlWindow, -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            SDL_Texture *fixedTexture = SDL_CreateTexture(
                sdlRenderer, SDL_PIXELFORMAT_ABGR8888,
                SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_Texture *floatTexture = SDL_CreateTexture(
                sdlRenderer, SDL_PIXELFORMAT_ABGR8888,
                SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

            while (!isExiting) {
                RendererTraceFrame(&floatRenderer, &game, floatBuffer);
                RendererTraceFrame(&fixedRenderer, &game, fixedBuffer);

                draw_buffer(sdlRenderer, fixedTexture, fixedBuffer, 0);
                draw_buffer(sdlRenderer, floatTexture, floatBuffer,
                            SCREEN_WIDTH + 1);

                SDL_RenderPresent(sdlRenderer);

                if (SDL_PollEvent(&event)) {
                    isExiting =
                        process_event(&event, &moveDirection, &rotateDirection);
                }
                const Uint64 nextCounter = SDL_GetPerformanceCounter();
                const Uint64 ticks = nextCounter - tickCounter;
                tickCounter = nextCounter;
                GameMove(&game, moveDirection, rotateDirection,
                         ticks / (SDL_GetPerformanceFrequency() >> 8));
            }
            SDL_DestroyTexture(floatTexture);
            SDL_DestroyTexture(fixedTexture);
            SDL_DestroyRenderer(sdlRenderer);
            SDL_DestroyWindow(sdlWindow);
        }
    }

    SDL_Quit();
    return 0;
}

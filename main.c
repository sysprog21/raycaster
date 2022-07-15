#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "game.h"
#include "raycaster.h"
#include "raycaster_fixed.h"
#include "raycaster_float.h"
#include "renderer.h"

static void DrawBuffer(SDL_Renderer *sdlRenderer,
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

static bool ProcessEvent(SDL_Event event,
                         int *moveDirection,
                         int *rotateDirection)
{
    if (event.type == SDL_QUIT) {
        return true;
    } else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) &&
               event.key.repeat == 0) {
        SDL_KeyboardEvent k = event.key;
        bool p = event.type == SDL_KEYDOWN;
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
            Game game = GameConstruct();
            RayCaster *floatCaster = RayCasterFloatConstruct();
            Renderer floatRenderer = RendererConstruct(floatCaster);
            uint32_t floatBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
            RayCaster *fixedCaster = RayCasterFixedConstruct();
            Renderer fixedRenderer = RendererConstruct(fixedCaster);
            uint32_t fixedBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
            int moveDirection = 0;
            int rotateDirection = 0;
            bool isExiting = false;
            const int tickFrequency = SDL_GetPerformanceFrequency();
            int tickCounter = SDL_GetPerformanceCounter();
            SDL_Event event;

            SDL_Renderer *sdlRenderer =
                SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
            SDL_Texture *fixedTexture = SDL_CreateTexture(
                sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
            SDL_Texture *floatTexture = SDL_CreateTexture(
                sdlRenderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

            float elapsed = 0.0f;
            int frames = 0;

            while (!isExiting) {
                RendererTraceFrame(&floatRenderer, &game, floatBuffer);
                RendererTraceFrame(&fixedRenderer, &game, fixedBuffer);

                DrawBuffer(sdlRenderer, fixedTexture, fixedBuffer, 0);
                DrawBuffer(sdlRenderer, floatTexture, floatBuffer,
                           SCREEN_WIDTH + 1);

                SDL_RenderPresent(sdlRenderer);

                if (SDL_PollEvent(&event)) {
                    isExiting =
                        ProcessEvent(event, &moveDirection, &rotateDirection);
                }
                const int nextCounter = SDL_GetPerformanceCounter();
                const float seconds =
                    (nextCounter - tickCounter) / ((float) tickFrequency);
                tickCounter = nextCounter;
                elapsed += seconds;
                if (elapsed >= 1.0f) {
                    char title[64];
                    sprintf(
                        title,
                        "RayCaster [fixed-point vs. floating-point] FPS: %.2f",
                        frames / elapsed);
                    SDL_SetWindowTitle(sdlWindow, title);
                    elapsed -= 1.0f;
                    frames = 0;
                }
                GameMove(&game, moveDirection, rotateDirection, seconds);
                ++frames;
            }
            SDL_DestroyTexture(floatTexture);
            SDL_DestroyTexture(fixedTexture);
            SDL_DestroyRenderer(sdlRenderer);
            SDL_DestroyWindow(sdlWindow);
            fixedCaster->Destruct(fixedCaster);
            floatCaster->Destruct(floatCaster);
        }
    }

    SDL_Quit();
    return 0;
}

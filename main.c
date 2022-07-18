#include <stdint.h>

#include "fb.h"
#include "game.h"
#include "mem.h"
#include "raycaster_fixed.h"
#include "renderer.h"
#include "uart.h"

void kmain()
{
    uint32_t *buffer = kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    RayCaster *rayCaster = RayCasterFixedConstruct();
    Game game = GameConstruct();
    Renderer renderer = RendererConstruct(rayCaster);
    RendererTraceFrame(&renderer, &game, buffer);
    rayCaster->Destruct(rayCaster);

    uint32_t *fb = fb_create(FB_WIDTH, FB_HEIGHT, 32);
    for (uint16_t x = 0; x < SCREEN_WIDTH; ++x) {
        for (uint16_t y = 0; y < SCREEN_HEIGHT; ++y) {
            uint32_t color = buffer[y * SCREEN_WIDTH + x];
            for (int i = 0; i < SCREEN_SCALE; ++i) {
                for (int j = 0; j < SCREEN_SCALE; ++j) {
                    fb[(y * SCREEN_SCALE + j) * FB_WIDTH +
                       (x * SCREEN_SCALE + i)] = color;
                }
            }
        }
    }

    kfree(buffer);
}
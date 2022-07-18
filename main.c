#include <stdint.h>

#include "fb.h"
#include "game.h"
#include "mem.h"
#include "raycaster_fixed.h"
#include "renderer.h"
#include "timer.h"
#include "uart.h"

void copy_buffer(uint32_t *fb, uint32_t *buffer)
{
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
}

void kmain()
{
    uint32_t *buffer = kmalloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
    RayCaster *rayCaster = RayCasterFixedConstruct();
    Game game = GameConstruct();
    Renderer renderer = RendererConstruct(rayCaster);

    uint32_t *fb = fb_create(FB_WIDTH, FB_HEIGHT, 32);
    uint64_t tickCounter = timer_clock();
    for (;;) {
        RendererTraceFrame(&renderer, &game, buffer);
        copy_buffer(fb, buffer);

        int m = 0, r = 0;
        if (!uart_empty()) {
            char c = uart_getc();
            switch (c) {
            case 'w':
                m = 1;
                break;
            case 'a':
                r = -1;
                break;
            case 's':
                m = -1;
                break;
            case 'd':
                r = 1;
                break;
            }
        }

        uint64_t nextCounter = timer_clock();
        uint64_t ticks = nextCounter - tickCounter;
        tickCounter = nextCounter;
        GameMove(&game, m, r, ticks >> 12);
    }

    rayCaster->Destruct(rayCaster);
    kfree(buffer);
}
#include <stdint.h>

#include "fb.h"
#include "game.h"
#include "mem.h"
#include "raycaster_fixed.h"
#include "renderer.h"
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
    for (;;) {
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

        GameMove(&game, m, r, 10);
        RendererTraceFrame(&renderer, &game, buffer);
        copy_buffer(fb, buffer);
    }

    rayCaster->Destruct(rayCaster);
    kfree(buffer);
}
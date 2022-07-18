#include <stdint.h>

#include "fb.h"
#include "game.h"
#include "mem.h"
#include "raycaster_fixed.h"
#include "renderer.h"
#include "uart.h"

void kmain()
{
    uint32_t *fb = fb_create(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
    RayCaster *rayCaster = RayCasterFixedConstruct();
    Game game = GameConstruct();
    Renderer renderer = RendererConstruct(rayCaster);
    RendererTraceFrame(&renderer, &game, fb);
    rayCaster->Destruct(rayCaster);
}
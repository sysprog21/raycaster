#include "renderer.h"
#include <math.h>
#include <stdlib.h>
#include "raycaster_data.h"

#define MULT_SCALAR_RGBA(scalar, rgba)                      \
    (((uint8_t) UMULT(scalar, (rgba >> 24) & 0xFF) << 24) | \
     ((uint8_t) UMULT(scalar, (rgba >> 16) & 0xFF) << 16) | \
     ((uint8_t) UMULT(scalar, (rgba >> 8) & 0xFF) << 8) |   \
     ((uint8_t) UMULT(scalar, (rgba >> 0) & 0xFF) << 0))

#define ADD_RGBA(rgba1, rgba2)                                     \
    ((((uint8_t) (rgba1 >> 24) + (uint8_t) (rgba2 >> 24)) << 24) | \
     (((uint8_t) (rgba1 >> 16) + (uint8_t) (rgba2 >> 16)) << 16) | \
     (((uint8_t) (rgba1 >> 8) + (uint8_t) (rgba2 >> 8)) << 8) |    \
     (((uint8_t) (rgba1 >> 0) + (uint8_t) (rgba2 >> 0)) << 0))

Renderer RendererConstruct(RayCaster *rc)
{
    Renderer renderer;
    renderer.rc = rc;
    return renderer;
}

void RendererTraceFrame(Renderer *renderer, Game *g, uint32_t *fb)
{
    renderer->rc->Start(renderer->rc, g->playerX, g->playerY, g->playerA);

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t sso;
        uint8_t tc;
        uint8_t tn;
        uint16_t tso;
        uint16_t tst;
        uint32_t *lb = fb + x;

        renderer->rc->Trace(renderer->rc, x, &sso, &tn, &tc, &tso, &tst);

        const int tx = (int) (tc >> 2);
        int16_t ws = HORIZON_HEIGHT - sso;
        if (ws < 0) {
            ws = 0;
            sso = HORIZON_HEIGHT;
        }
        uint16_t to = tso;
        uint16_t ts = tst;

        for (int y = 0; y < ws; y++) {
            *lb = ADD_RGBA(
                MULT_SCALAR_RGBA(96 + (HORIZON_HEIGHT - y), 0xFFFFB380),
                MULT_SCALAR_RGBA(255 - (96 + (HORIZON_HEIGHT - y)),
                                 0xFFFFFFFF));
            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < sso * 2; y++) {
            // paint texture pixel
            int ty = (int) (to >> 10);
            uint32_t tv = g_texture32[(ty << 6) + tx];

            to += ts;

            if (tn == 1 && tv > 0) {
                // dark wall
                tv = ((uint8_t) (tv >> 24) << 24) |
                     (((uint8_t) (tv >> 16) >> 1) << 16) |
                     (((uint8_t) (tv >> 8) >> 1) << 8) |
                     (((uint8_t) (tv >> 0) >> 1) << 0);
            }
            *lb = tv;
            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < ws; y++) {
            *lb = ADD_RGBA(
                MULT_SCALAR_RGBA(96 + (HORIZON_HEIGHT - (ws - y)), 0xFF53769B),
                MULT_SCALAR_RGBA(255 - (96 + (HORIZON_HEIGHT - (ws - y))),
                                 0xFFFFFFFF));
            lb += SCREEN_WIDTH;
        }
    }
}

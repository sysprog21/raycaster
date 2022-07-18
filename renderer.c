#include "renderer.h"
#include <math.h>
#include <stdlib.h>
#include "raycaster_data.h"

inline static uint32_t MultScalarRGBA(uint16_t scalar, uint32_t rgba)
{
    return ((uint8_t) UMULT(scalar, (rgba >> 24) & 0xFF) << 24) |
           ((uint8_t) UMULT(scalar, (rgba >> 16) & 0xFF) << 16) |
           ((uint8_t) UMULT(scalar, (rgba >> 8) & 0xFF) << 8) |
           ((uint8_t) UMULT(scalar, (rgba >> 0) & 0xFF) << 0);
}

inline static uint32_t AddRGBARGBA(uint32_t rgba1, uint32_t rgba2)
{
    return (((uint8_t) (rgba1 >> 24) + (uint8_t) (rgba2 >> 24)) << 24) |
           (((uint8_t) (rgba1 >> 16) + (uint8_t) (rgba2 >> 16)) << 16) |
           (((uint8_t) (rgba1 >> 8) + (uint8_t) (rgba2 >> 8)) << 8) |
           (((uint8_t) (rgba1 >> 0) + (uint8_t) (rgba2 >> 0)) << 0);
}

inline static uint32_t GetRGBA(uint8_t brightness)
{
    return (brightness << 16) + (brightness << 8) + brightness;
}

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
            *lb = AddRGBARGBA(
                MultScalarRGBA(96 + (HORIZON_HEIGHT - y), 0xFFFFB380),
                MultScalarRGBA(255 - (96 + (HORIZON_HEIGHT - y)), 0xFFFFFFFF));
            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < sso * 2; y++) {
            // paint texture pixel
            int ty = (int) (to >> 10);
            uint8_t tv = g_texture8[(ty << 6) + tx];

            to += ts;

            if (tn == 1 && tv > 0) {
                // dark wall
                tv >>= 1;
            }
            *lb = GetRGBA(tv);
            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < ws; y++) {
            *lb = AddRGBARGBA(
                MultScalarRGBA(96 + (HORIZON_HEIGHT - (ws - y)), 0xFF53769B),
                MultScalarRGBA(255 - (96 + (HORIZON_HEIGHT - (ws - y))),
                               0xFFFFFFFF));
            lb += SCREEN_WIDTH;
        }
    }
}

#include "renderer.h"
#include <math.h>
#include <stdlib.h>
#include "raycaster_data.h"

inline static uint32_t GetARGB(uint8_t brightness)
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
    RayCasterStart(renderer->rc, g->playerX, g->playerY, g->playerA);

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        uint8_t sso;
        uint8_t tc;
        uint8_t tn;
        uint16_t tso;
        uint16_t tst;
        uint32_t *lb = fb + x;

        RayCasterTrace(renderer->rc, x, &sso, &tn, &tc, &tso, &tst);

        const int tx = (int) (tc >> 2);
        int16_t ws = HORIZON_HEIGHT - sso;
        if (ws < 0) {
            ws = 0;
            sso = HORIZON_HEIGHT;
        }
        uint16_t to = tso;
        uint16_t ts = tst;

        for (int y = 0; y < ws; y++) {
            *lb = GetARGB(96 + (HORIZON_HEIGHT - y));
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
            *lb = GetARGB(tv);
            lb += SCREEN_WIDTH;
        }

        for (int y = 0; y < ws; y++) {
            *lb = GetARGB(96 + (HORIZON_HEIGHT - (ws - y)));
            lb += SCREEN_WIDTH;
        }
    }
}

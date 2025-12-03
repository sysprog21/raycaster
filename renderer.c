#include "renderer.h"
#include <math.h>
#include <stdlib.h>
#include "raycaster_data.h"

#define MULT_SCALAR_RGBA(scalar, rgba)                      \
    (((uint8_t) UMULT(scalar, (rgba >> 24) & 0xFF) << 24) | \
     ((uint8_t) UMULT(scalar, (rgba >> 16) & 0xFF) << 16) | \
     ((uint8_t) UMULT(scalar, (rgba >> 8) & 0xFF) << 8) |   \
     ((uint8_t) UMULT(scalar, (rgba >> 0) & 0xFF) << 0))

/* Branchless saturating add for packed RGBA (SWAR technique).
 * Detects per-byte overflow and saturates to 0xFF without branches.
 * Much faster than per-channel MIN in hot pixel loops.
 */
static inline uint32_t add_rgba(uint32_t a, uint32_t b)
{
    uint32_t sum = a + b;
    /* Detect per-byte overflow: high bit set where carry occurred */
    uint32_t carry = ((a & b) | ((a | b) & ~sum)) & 0x80808080u;
    /* Expand overflow bits to 0x7F mask, then OR to saturate to 0xFF */
    carry = (carry - (carry >> 7)) & 0x7F7F7F7Fu;
    return sum | carry;
}

#define ADD_RGBA(a, b) add_rgba((a), (b))

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
            // paint texture pixel (mask ty to 0-63 to prevent out-of-bounds)
            int ty = (int) (to >> 10) & 63;
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

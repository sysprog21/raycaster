#include "renderer.h"
#include <math.h>
#include <stdlib.h>
#include "raycaster_data.h"

/* 3x5 pixel digit font (0-9), each digit packed into uint16_t (15 bits used) */
static const uint16_t g_digitFont[10] = {
    0x7B6F, /* 0: 111 101 101 101 111 */
    0x2492, /* 1: 010 010 010 010 010 */
    0x73E7, /* 2: 111 001 111 100 111 */
    0x73CF, /* 3: 111 001 111 001 111 */
    0x5BC9, /* 4: 101 101 111 001 001 */
    0x79CF, /* 5: 111 100 111 001 111 */
    0x79EF, /* 6: 111 100 111 101 111 */
    0x7249, /* 7: 111 001 001 001 001 */
    0x7BEF, /* 8: 111 101 111 101 111 */
    0x7BCF, /* 9: 111 101 111 001 111 */
};

#define FPS_COLOR 0xFF00FF00  /* Green */
#define FPS_SHADOW 0xFF000000 /* Black shadow */
#define DIGIT_WIDTH 4         /* 3 pixels + 1 spacing */
#define DIGIT_HEIGHT 5

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

/* Draw a single digit at (x, y) position */
static void draw_digit(uint32_t *fb, int x, int y, int digit, uint32_t color)
{
    if (digit < 0 || digit > 9)
        return;
    uint16_t glyph = g_digitFont[digit];
    for (int row = 0; row < DIGIT_HEIGHT; row++) {
        for (int col = 0; col < 3; col++) {
            int px = x + col;
            int py = y + row;
            if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                /* Bit index: row 0 is bits 14-12, row 1 is bits 11-9, etc. */
                int bitIdx = (4 - row) * 3 + (2 - col);
                if (glyph & (1 << bitIdx))
                    fb[py * SCREEN_WIDTH + px] = color;
            }
        }
    }
}

void RendererDrawFPS(uint32_t *fb, uint32_t fps)
{
    /* Position: top-right corner (DIGIT_WIDTH includes 1px spacing) */
    int startX = SCREEN_WIDTH - 1;
    int startY = 2;

    /* Count digits to determine starting position */
    uint32_t temp = fps;
    int numDigits = 0;
    do {
        numDigits++;
        temp /= 10;
    } while (temp > 0);

    /* Calculate left-most digit position */
    startX -= numDigits * DIGIT_WIDTH;

    /* Draw digits from left to right */
    temp = fps;
    int digits[10];
    for (int i = numDigits - 1; i >= 0; i--) {
        digits[i] = temp % 10;
        temp /= 10;
    }

    for (int i = 0; i < numDigits; i++) {
        int x = startX + i * DIGIT_WIDTH;
        /* Draw shadow first (offset by 1 pixel) */
        draw_digit(fb, x + 1, startY + 1, digits[i], FPS_SHADOW);
        /* Draw digit */
        draw_digit(fb, x, startY, digits[i], FPS_COLOR);
    }
}

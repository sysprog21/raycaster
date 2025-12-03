#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "raycaster.h"

static uint16_t g_tan[256];
static uint16_t g_cotan[256];
static uint8_t g_sin[256];
static uint8_t g_cos[256];
static uint8_t g_nearHeight[256];
static uint8_t g_farHeight[256];
static uint16_t g_nearStep[256];
static uint16_t g_farStep[256];
static uint16_t g_overflowOffset[256];
static uint16_t g_overflowStep[256];
static uint16_t g_deltaAngle[SCREEN_WIDTH];

static void dump_u8_table(const char *name, const uint8_t *t, int len)
{
    printf("const uint8_t LOOKUP_TBL %s[%d] = {", name, len);
    for (int i = 0; i < len; i++) {
        printf("%u", t[i]);
        if (i < len - 1)
            printf(",");
    }
    printf("};\n\n");
}

static void dump_u16_table(const char *name, const uint16_t *t, int len)
{
    printf("const uint16_t LOOKUP_TBL %s[%d] = {", name, len);
    for (int i = 0; i < len; i++) {
        printf("%u", t[i]);
        if (i < len - 1)
            printf(",");
    }
    printf("};\n\n");
}

static void precalculate(void)
{
    for (int i = 0; i < 256; i++) {
        g_tan[i] = (uint16_t) (256.0f * tanf(i * (float) M_PI_2 / 256.0f));
        g_cotan[i] = (uint16_t) (256.0f / tanf(i * (float) M_PI_2 / 256.0f));
    }

    for (int i = 0; i < 256; i++) {
        float s = 256.0f * sinf(i / 1024.0f * 2 * (float) M_PI);
        float c = 256.0f * cosf(i / 1024.0f * 2 * (float) M_PI);
        g_sin[i] = (uint8_t) fminf(s, 255.0f);
        g_cos[i] = (uint8_t) fminf(c, 255.0f);
    }

    for (int i = 0; i < SCREEN_WIDTH; i++) {
        float deltaAngle = atanf(((int16_t) i - SCREEN_WIDTH / 2.0f) /
                                 (SCREEN_WIDTH / 2.0f) * (float) M_PI / 4);
        int16_t da = (int16_t) (deltaAngle / (float) M_PI_2 * 256.0f);
        if (da < 0)
            da += 1024;
        g_deltaAngle[i] = (uint16_t) da;
    }

    for (int i = 0; i < 256; i++) {
        g_nearHeight[i] =
            (uint8_t) ((INV_FACTOR_INT / (((i << 2) + MIN_DIST) >> 2)) >> 2);
        g_farHeight[i] =
            (uint8_t) ((INV_FACTOR_INT / (((i << 5) + MIN_DIST) >> 5)) >> 5);
    }

    for (int i = 0; i < 256; i++) {
        float txn =
            ((INV_FACTOR_INT / (((i * 4.0f) + MIN_DIST) / 4.0f)) / 4.0f) * 2.0f;
        if (txn != 0)
            g_nearStep[i] = (uint16_t) ((256 / txn) * 256);
        float txf =
            ((INV_FACTOR_INT / (((i * 32.0f) + MIN_DIST) / 32.0f)) / 32.0f) *
            2.0f;
        if (txf != 0)
            g_farStep[i] = (uint16_t) ((256 / txf) * 256);
    }

    for (int i = 1; i < 256; i++) {
        float txs = INV_FACTOR_INT / (i / 2.0f);
        float ino = (txs - SCREEN_HEIGHT) / 2;
        g_overflowStep[i] = (uint16_t) ((256 / txs) * 256);
        g_overflowOffset[i] = (uint16_t) (ino * (256 / txs) * 256);
    }

    dump_u16_table("g_tan", g_tan, 256);
    dump_u16_table("g_cotan", g_cotan, 256);
    dump_u8_table("g_sin", g_sin, 256);
    dump_u8_table("g_cos", g_cos, 256);
    dump_u8_table("g_nearHeight", g_nearHeight, 256);
    dump_u8_table("g_farHeight", g_farHeight, 256);
    dump_u16_table("g_nearStep", g_nearStep, 256);
    dump_u16_table("g_farStep", g_farStep, 256);
    dump_u16_table("g_overflowOffset", g_overflowOffset, 256);
    dump_u16_table("g_overflowStep", g_overflowStep, 256);
    dump_u16_table("g_deltaAngle", g_deltaAngle, SCREEN_WIDTH);
}

int main(void)
{
    printf("#include \"raycaster.h\"\n\n");
    precalculate();
    return 0;
}

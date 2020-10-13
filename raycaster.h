#pragma once

#include <stdint.h>

#define TABLES_320
#define SCREEN_WIDTH (uint16_t) 320
#define SCREEN_HEIGHT (uint16_t) 256
#define SCREEN_SCALE 2
#define FOV (double) (M_PI / 2)
#define INV_FACTOR (float) (SCREEN_WIDTH * 95.0f / 320.0f)
#define LOOKUP_TBL
#define LOOKUP8(tbl, offset) tbl[offset]
#define LOOKUP16(tbl, offset) tbl[offset]

#define MAP_X (uint8_t) 32
#define MAP_XS (uint8_t) 5
#define MAP_Y (uint8_t) 32
#define INV_FACTOR_INT ((uint16_t)(SCREEN_WIDTH * 75))
#define MIN_DIST (int) ((150 * ((float) SCREEN_WIDTH / (float) SCREEN_HEIGHT)))
#define HORIZON_HEIGHT (SCREEN_HEIGHT / 2)
#define INVERT(x) (uint8_t)((x ^ 255) + 1)
#define ABS(x) (x < 0 ? -x : x)

class RayCaster
{
public:
    virtual void Start(uint16_t playerX, uint16_t playerY, int16_t playerA) = 0;

    virtual void Trace(uint16_t screenX,
                       uint8_t *screenY,
                       uint8_t *textureNo,
                       uint8_t *textureX,
                       uint16_t *textureY,
                       uint16_t *textureStep) = 0;

    RayCaster();

    ~RayCaster();
};

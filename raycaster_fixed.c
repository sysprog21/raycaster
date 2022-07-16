// fixed-point implementation

#include "raycaster_fixed.h"
#include <stdbool.h>
#include <stdlib.h>
#include "raycaster_data.h"
#include "raycaster_tables.h"

#define RayCasterFixed struct RayCasterFixed_

struct RayCasterFixed_ {
    uint16_t playerX;
    uint16_t playerY;
    int16_t playerA;
    uint8_t viewQuarter;
    uint8_t viewAngle;
};

static void RayCasterFixedStart(RayCaster *rayCaster,
                                uint16_t playerX,
                                uint16_t playerY,
                                int16_t playerA);
static void RayCasterFixedTrace(RayCaster *rayCaster,
                                uint16_t screenX,
                                uint8_t *screenY,
                                uint8_t *textureNo,
                                uint8_t *textureX,
                                uint16_t *textureY,
                                uint16_t *textureStep);
static void RayCasterFixedDestruct(RayCaster *rayCaster);

RayCaster *RayCasterFixedConstruct(void)
{
    RayCaster *rayCaster = RayCasterConstruct();
    RayCasterFixed *rayCasterFixed =
        (RayCasterFixed *) malloc(sizeof(RayCasterFixed));
    if (!rayCasterFixed) {
        rayCaster->Destruct(rayCaster);
        return NULL;
    }
    rayCaster->derived = rayCasterFixed;

    rayCaster->Start = RayCasterFixedStart;
    rayCaster->Trace = RayCasterFixedTrace;
    rayCaster->Destruct = RayCasterFixedDestruct;

    return rayCaster;
}

// (v * f) >> 8
static uint16_t RayCasterFixedMulU(uint8_t v, uint16_t f)
{
    const uint8_t f_h = f >> 8;
    const uint8_t f_l = f % 256;
    const uint16_t hm = v * f_h;
    const uint16_t lm = v * f_l;
    return hm + (lm >> 8);
}

static int16_t RayCasterFixedMulS(uint8_t v, int16_t f)
{
    const uint16_t uf = RayCasterFixedMulU(v, (uint16_t) ABS(f));
    if (f < 0) {
        return ~uf;
    }
    return uf;
}

inline int16_t RayCasterFixedAbsTan(uint8_t quarter,
                                    uint8_t angle,
                                    const uint16_t *lookupTable)
{
    if (quarter & 1) {
        return LOOKUP16(lookupTable, INVERT(angle));
    }
    return LOOKUP16(lookupTable, angle);
}

static int16_t RayCasterFixedMulTan(uint8_t value,
                                    bool inverse,
                                    uint8_t quarter,
                                    uint8_t angle,
                                    const uint16_t *lookupTable)
{
    uint8_t signedValue = value;
    if (inverse) {
        if (value == 0) {
            if (quarter % 2 == 1) {
                return -RayCasterFixedAbsTan(quarter, angle, lookupTable);
            }
            return RayCasterFixedAbsTan(quarter, angle, lookupTable);
        }
        signedValue = INVERT(value);
    }
    if (signedValue == 0) {
        return 0;
    }
    if (quarter % 2 == 1) {
        return -RayCasterFixedMulU(signedValue,
                                   LOOKUP16(lookupTable, INVERT(angle)));
    }
    return RayCasterFixedMulU(signedValue, LOOKUP16(lookupTable, angle));
}

inline bool RayCasterFixedIsWall(uint8_t tileX, uint8_t tileY)
{
    if (tileX >= MAP_X - 1 || tileY >= MAP_Y - 1) {
        return true;
    }
    return LOOKUP8(g_map, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
           (1 << (8 - (tileX & 0x7)));
}

static void RayCasterFixedLookupHeight(uint16_t distance,
                                       uint8_t *height,
                                       uint16_t *step)
{
    if (distance >= 256) {
        const uint16_t ds = distance >> 3;
        if (ds >= 256) {
            *height = LOOKUP8(g_farHeight, 255) - 1;
            *step = LOOKUP16(g_farStep, 255);
        } else {
            *height = LOOKUP8(g_farHeight, ds);
            *step = LOOKUP16(g_farStep, ds);
        }
    } else {
        *height = LOOKUP8(g_nearHeight, distance);
        *step = LOOKUP16(g_nearStep, distance);
    }
}

static void RayCasterFixedCalculateDistance(uint16_t rayX,
                                            uint16_t rayY,
                                            uint16_t rayA,
                                            int16_t *deltaX,
                                            int16_t *deltaY,
                                            uint8_t *textureNo,
                                            uint8_t *textureX)
{
    int8_t tileStepX = 0;
    int8_t tileStepY = 0;
    int16_t interceptX = rayX;
    int16_t interceptY = rayY;

    const uint8_t quarter = rayA >> 8;
    const uint8_t angle = rayA % 256;
    const uint8_t offsetX = rayX % 256;
    const uint8_t offsetY = rayY % 256;

    uint8_t tileX = rayX >> 8;
    uint8_t tileY = rayY >> 8;
    int16_t hitX;
    int16_t hitY;

    if (angle == 0) {
        switch (quarter % 2) {
        case 0:
            tileStepX = 0;
            tileStepY = quarter == 0 ? 1 : -1;
            if (tileStepY == 1) {
                interceptY -= 256;
            }
            for (;;) {
                tileY += tileStepY;
                if (RayCasterFixedIsWall(tileX, tileY)) {
                    goto HorizontalHit;
                }
            }
            break;
        case 1:
            tileStepY = 0;
            tileStepX = quarter == 1 ? 1 : -1;
            if (tileStepX == 1) {
                interceptX -= 256;
            }
            for (;;) {
                tileX += tileStepX;
                if (RayCasterFixedIsWall(tileX, tileY)) {
                    goto VerticalHit;
                }
            }
            break;
        }
    } else {
        int16_t stepX = 0;
        int16_t stepY = 0;

        switch (quarter) {
        case 0:
        case 1:
            tileStepX = 1;
            interceptY +=
                RayCasterFixedMulTan(offsetX, true, quarter, angle, g_cotan);
            interceptX -= 256;
            stepX = RayCasterFixedAbsTan(quarter, angle, g_tan);
            break;
        case 2:
        case 3:
            tileStepX = -1;
            interceptY -=
                RayCasterFixedMulTan(offsetX, false, quarter, angle, g_cotan);
            stepX = -RayCasterFixedAbsTan(quarter, angle, g_tan);
            break;
        }

        switch (quarter) {
        case 0:
        case 3:
            tileStepY = 1;
            interceptX +=
                RayCasterFixedMulTan(offsetY, true, quarter, angle, g_tan);
            interceptY -= 256;
            stepY = RayCasterFixedAbsTan(quarter, angle, g_cotan);
            break;
        case 1:
        case 2:
            tileStepY = -1;
            interceptX -=
                RayCasterFixedMulTan(offsetY, false, quarter, angle, g_tan);
            stepY = -RayCasterFixedAbsTan(quarter, angle, g_cotan);
            break;
        }

        for (;;) {
            while ((tileStepY == 1 && (interceptY >> 8 < tileY)) ||
                   (tileStepY == -1 && (interceptY >> 8 >= tileY))) {
                tileX += tileStepX;
                if (RayCasterFixedIsWall(tileX, tileY)) {
                    goto VerticalHit;
                }
                interceptY += stepY;
            }
            while ((tileStepX == 1 && (interceptX >> 8 < tileX)) ||
                   (tileStepX == -1 && (interceptX >> 8 >= tileX))) {
                tileY += tileStepY;
                if (RayCasterFixedIsWall(tileX, tileY)) {
                    goto HorizontalHit;
                }
                interceptX += stepX;
            }
        }
    }

HorizontalHit:
    hitX = interceptX + (tileStepX == 1 ? 256 : 0);
    hitY = (tileY << 8) + (tileStepY == -1 ? 256 : 0);
    *textureNo = 0;
    *textureX = interceptX & 0xFF;
    goto WallHit;

VerticalHit:
    hitX = (tileX << 8) + (tileStepX == -1 ? 256 : 0);
    hitY = interceptY + (tileStepY == 1 ? 256 : 0);
    *textureNo = 1;
    *textureX = interceptY & 0xFF;
    goto WallHit;

WallHit:
    *deltaX = hitX - rayX;
    *deltaY = hitY - rayY;
}

static void RayCasterFixedStart(RayCaster *rayCaster,
                                uint16_t playerX,
                                uint16_t playerY,
                                int16_t playerA)
{
    ((RayCasterFixed *) (rayCaster->derived))->viewQuarter = playerA >> 8;
    ((RayCasterFixed *) (rayCaster->derived))->viewAngle = playerA % 256;
    ((RayCasterFixed *) (rayCaster->derived))->playerX = playerX;
    ((RayCasterFixed *) (rayCaster->derived))->playerY = playerY;
    ((RayCasterFixed *) (rayCaster->derived))->playerA = playerA;
}

// (playerX, playerY) is 8 box coordinate bits, 8 inside coordinate bits
// (playerA) is full circle as 1024
static void RayCasterFixedTrace(RayCaster *rayCaster,
                                uint16_t screenX,
                                uint8_t *screenY,
                                uint8_t *textureNo,
                                uint8_t *textureX,
                                uint16_t *textureY,
                                uint16_t *textureStep)
{
    uint16_t rayAngle =
        (uint16_t) (((RayCasterFixed *) (rayCaster->derived))->playerA +
                    LOOKUP16(g_deltaAngle, screenX));

    // neutralize artefacts around edges
    switch (rayAngle % 256) {
    case 1:
    case 254:
        rayAngle--;
        break;
    case 2:
    case 255:
        rayAngle++;
        break;
    }
    rayAngle %= 1024;

    int16_t deltaX;
    int16_t deltaY;
    RayCasterFixedCalculateDistance(
        ((RayCasterFixed *) (rayCaster->derived))->playerX,
        ((RayCasterFixed *) (rayCaster->derived))->playerY, rayAngle, &deltaX,
        &deltaY, textureNo, textureX);

    // distance = deltaY * cos(playerA) + deltaX * sin(playerA)
    int16_t distance = 0;
    if (((RayCasterFixed *) (rayCaster->derived))->playerA == 0) {
        distance += deltaY;
    } else if (((RayCasterFixed *) (rayCaster->derived))->playerA == 512) {
        distance -= deltaY;
    } else
        switch (((RayCasterFixed *) (rayCaster->derived))->viewQuarter) {
        case 0:
            distance += RayCasterFixedMulS(
                LOOKUP8(g_cos,
                        ((RayCasterFixed *) (rayCaster->derived))->viewAngle),
                deltaY);
            break;
        case 1:
            distance -= RayCasterFixedMulS(
                LOOKUP8(g_cos, INVERT(((RayCasterFixed *) (rayCaster->derived))
                                          ->viewAngle)),
                deltaY);
            break;
        case 2:
            distance -= RayCasterFixedMulS(
                LOOKUP8(g_cos,
                        ((RayCasterFixed *) (rayCaster->derived))->viewAngle),
                deltaY);
            break;
        case 3:
            distance += RayCasterFixedMulS(
                LOOKUP8(g_cos, INVERT(((RayCasterFixed *) (rayCaster->derived))
                                          ->viewAngle)),
                deltaY);
            break;
        }

    if (((RayCasterFixed *) (rayCaster->derived))->playerA == 256) {
        distance += deltaX;
    } else if (((RayCasterFixed *) (rayCaster->derived))->playerA == 768) {
        distance -= deltaX;
    } else
        switch (((RayCasterFixed *) (rayCaster->derived))->viewQuarter) {
        case 0:
            distance += RayCasterFixedMulS(
                LOOKUP8(g_sin,
                        ((RayCasterFixed *) (rayCaster->derived))->viewAngle),
                deltaX);
            break;
        case 1:
            distance += RayCasterFixedMulS(
                LOOKUP8(g_sin, INVERT(((RayCasterFixed *) (rayCaster->derived))
                                          ->viewAngle)),
                deltaX);
            break;
        case 2:
            distance -= RayCasterFixedMulS(
                LOOKUP8(g_sin,
                        ((RayCasterFixed *) (rayCaster->derived))->viewAngle),
                deltaX);
            break;
        case 3:
            distance -= RayCasterFixedMulS(
                LOOKUP8(g_sin, INVERT(((RayCasterFixed *) (rayCaster->derived))
                                          ->viewAngle)),
                deltaX);
            break;
        }
    if (distance >= MIN_DIST) {
        *textureY = 0;
        RayCasterFixedLookupHeight((distance - MIN_DIST) >> 2, screenY,
                                   textureStep);
    } else {
        *screenY = SCREEN_HEIGHT >> 1;
        *textureY = LOOKUP16(g_overflowOffset, distance);
        *textureStep = LOOKUP16(g_overflowStep, distance);
    }
}

static void RayCasterFixedDestruct(RayCaster *rayCaster)
{
    free(rayCaster->derived);
    RayCasterDestruct(rayCaster);
}
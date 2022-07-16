#include <stdlib.h>

// fixed-point implementation

#include <stdbool.h>
#include <stdlib.h>
#include "raycaster.h"
#include "raycaster_data.h"
#include "raycaster_tables.h"

RayCaster RayCasterConstruct(void)
{
    RayCaster rayCaster = {};
    return rayCaster;
}

// (v * f) >> 8
static uint16_t RayCasterMulU(uint8_t v, uint16_t f)
{
    const uint8_t f_h = f >> 8;
    const uint8_t f_l = f % 256;
    const uint16_t hm = v * f_h;
    const uint16_t lm = v * f_l;
    return hm + (lm >> 8);
}

static int16_t RayCasterMulS(uint8_t v, int16_t f)
{
    const uint16_t uf = RayCasterMulU(v, (uint16_t) ABS(f));
    if (f < 0) {
        return ~uf;
    }
    return uf;
}

inline int16_t RayCasterAbsTan(uint8_t quarter,
                               uint8_t angle,
                               const uint16_t *lookupTable)
{
    if (quarter & 1) {
        return LOOKUP16(lookupTable, INVERT(angle));
    }
    return LOOKUP16(lookupTable, angle);
}

static int16_t RayCasterMulTan(uint8_t value,
                               bool inverse,
                               uint8_t quarter,
                               uint8_t angle,
                               const uint16_t *lookupTable)
{
    uint8_t signedValue = value;
    if (inverse) {
        if (value == 0) {
            if (quarter % 2 == 1) {
                return -RayCasterAbsTan(quarter, angle, lookupTable);
            }
            return RayCasterAbsTan(quarter, angle, lookupTable);
        }
        signedValue = INVERT(value);
    }
    if (signedValue == 0) {
        return 0;
    }
    if (quarter % 2 == 1) {
        return -RayCasterMulU(signedValue,
                              LOOKUP16(lookupTable, INVERT(angle)));
    }
    return RayCasterMulU(signedValue, LOOKUP16(lookupTable, angle));
}

inline bool RayCasterIsWall(uint8_t tileX, uint8_t tileY)
{
    if (tileX >= MAP_X - 1 || tileY >= MAP_Y - 1) {
        return true;
    }
    return LOOKUP8(g_map, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
           (1 << (8 - (tileX & 0x7)));
}

static void RayCasterLookupHeight(uint16_t distance,
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

static void RayCasterCalculateDistance(uint16_t rayX,
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
                if (RayCasterIsWall(tileX, tileY)) {
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
                if (RayCasterIsWall(tileX, tileY)) {
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
                RayCasterMulTan(offsetX, true, quarter, angle, g_cotan);
            interceptX -= 256;
            stepX = RayCasterAbsTan(quarter, angle, g_tan);
            break;
        case 2:
        case 3:
            tileStepX = -1;
            interceptY -=
                RayCasterMulTan(offsetX, false, quarter, angle, g_cotan);
            stepX = -RayCasterAbsTan(quarter, angle, g_tan);
            break;
        }

        switch (quarter) {
        case 0:
        case 3:
            tileStepY = 1;
            interceptX += RayCasterMulTan(offsetY, true, quarter, angle, g_tan);
            interceptY -= 256;
            stepY = RayCasterAbsTan(quarter, angle, g_cotan);
            break;
        case 1:
        case 2:
            tileStepY = -1;
            interceptX -=
                RayCasterMulTan(offsetY, false, quarter, angle, g_tan);
            stepY = -RayCasterAbsTan(quarter, angle, g_cotan);
            break;
        }

        for (;;) {
            while ((tileStepY == 1 && (interceptY >> 8 < tileY)) ||
                   (tileStepY == -1 && (interceptY >> 8 >= tileY))) {
                tileX += tileStepX;
                if (RayCasterIsWall(tileX, tileY)) {
                    goto VerticalHit;
                }
                interceptY += stepY;
            }
            while ((tileStepX == 1 && (interceptX >> 8 < tileX)) ||
                   (tileStepX == -1 && (interceptX >> 8 >= tileX))) {
                tileY += tileStepY;
                if (RayCasterIsWall(tileX, tileY)) {
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

void RayCasterStart(RayCaster *rayCaster,
                    uint16_t playerX,
                    uint16_t playerY,
                    int16_t playerA)
{
    rayCaster->viewQuarter = playerA >> 8;
    rayCaster->viewAngle = playerA % 256;
    rayCaster->playerX = playerX;
    rayCaster->playerY = playerY;
    rayCaster->playerA = playerA;
}

// (playerX, playerY) is 8 box coordinate bits, 8 inside coordinate bits
// (playerA) is full circle as 1024
void RayCasterTrace(RayCaster *rayCaster,
                    uint16_t screenX,
                    uint8_t *screenY,
                    uint8_t *textureNo,
                    uint8_t *textureX,
                    uint16_t *textureY,
                    uint16_t *textureStep)
{
    uint16_t rayAngle =
        (uint16_t) (rayCaster->playerA + LOOKUP16(g_deltaAngle, screenX));

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
    RayCasterCalculateDistance(rayCaster->playerX, rayCaster->playerY, rayAngle,
                               &deltaX, &deltaY, textureNo, textureX);

    // distance = deltaY * cos(playerA) + deltaX * sin(playerA)
    int16_t distance = 0;
    if (rayCaster->playerA == 0) {
        distance += deltaY;
    } else if (rayCaster->playerA == 512) {
        distance -= deltaY;
    } else
        switch (rayCaster->viewQuarter) {
        case 0:
            distance +=
                RayCasterMulS(LOOKUP8(g_cos, rayCaster->viewAngle), deltaY);
            break;
        case 1:
            distance -= RayCasterMulS(
                LOOKUP8(g_cos, INVERT(rayCaster->viewAngle)), deltaY);
            break;
        case 2:
            distance -=
                RayCasterMulS(LOOKUP8(g_cos, rayCaster->viewAngle), deltaY);
            break;
        case 3:
            distance += RayCasterMulS(
                LOOKUP8(g_cos, INVERT(rayCaster->viewAngle)), deltaY);
            break;
        }

    if (rayCaster->playerA == 256) {
        distance += deltaX;
    } else if (rayCaster->playerA == 768) {
        distance -= deltaX;
    } else
        switch (rayCaster->viewQuarter) {
        case 0:
            distance +=
                RayCasterMulS(LOOKUP8(g_sin, rayCaster->viewAngle), deltaX);
            break;
        case 1:
            distance += RayCasterMulS(
                LOOKUP8(g_sin, INVERT(rayCaster->viewAngle)), deltaX);
            break;
        case 2:
            distance -=
                RayCasterMulS(LOOKUP8(g_sin, rayCaster->viewAngle), deltaX);
            break;
        case 3:
            distance -= RayCasterMulS(
                LOOKUP8(g_sin, INVERT(rayCaster->viewAngle)), deltaX);
            break;
        }
    if (distance >= MIN_DIST) {
        *textureY = 0;
        RayCasterLookupHeight((distance - MIN_DIST) >> 2, screenY, textureStep);
    } else {
        *screenY = SCREEN_HEIGHT >> 1;
        *textureY = LOOKUP16(g_overflowOffset, distance);
        *textureStep = LOOKUP16(g_overflowStep, distance);
    }
}
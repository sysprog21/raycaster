// fixed-point implementation

#include "raycaster_fixed.h"
#include "raycaster_data.h"
#include "raycaster_tables.h"

// (v * f) >> 8
uint16_t RayCasterFixed::MulU(uint8_t v, uint16_t f)
{
    const uint8_t f_h = f >> 8;
    const uint8_t f_l = f % 256;
    const uint16_t hm = v * f_h;
    const uint16_t lm = v * f_l;
    return hm + (lm >> 8);
}

int16_t RayCasterFixed::MulS(uint8_t v, int16_t f)
{
    const uint16_t uf = MulU(v, static_cast<uint16_t>(ABS(f)));
    if (f < 0) {
        return ~uf;
    }
    return uf;
}

int16_t RayCasterFixed::MulTan(uint8_t value,
                               bool inverse,
                               uint8_t quarter,
                               uint8_t angle,
                               const uint16_t *lookupTable)
{
    uint8_t signedValue = value;
    if (inverse) {
        if (value == 0) {
            if (quarter % 2 == 1) {
                return -AbsTan(quarter, angle, lookupTable);
            }
            return AbsTan(quarter, angle, lookupTable);
        }
        signedValue = INVERT(value);
    }
    if (signedValue == 0) {
        return 0;
    }
    if (quarter % 2 == 1) {
        return -MulU(signedValue, LOOKUP16(lookupTable, INVERT(angle)));
    }
    return MulU(signedValue, LOOKUP16(lookupTable, angle));
}

inline int16_t RayCasterFixed::AbsTan(uint8_t quarter,
                                      uint8_t angle,
                                      const uint16_t *lookupTable)
{
    if (quarter & 1) {
        return LOOKUP16(lookupTable, INVERT(angle));
    }
    return LOOKUP16(lookupTable, angle);
}

void RayCasterFixed::LookupHeight(uint16_t distance,
                                  uint8_t *height,
                                  uint16_t *step)
{
    if (distance >= 256) {
        const uint16_t ds = distance >> 3;
        if (ds >= 256) {
            *height = LOOKUP8(g_farHeight, 255) - 1;
            *step = LOOKUP16(g_farStep, 255);
        }
        *height = LOOKUP8(g_farHeight, ds);
        *step = LOOKUP16(g_farStep, ds);
    } else {
        *height = LOOKUP8(g_nearHeight, distance);
        *step = LOOKUP16(g_nearStep, distance);
    }
}

void RayCasterFixed::CalculateDistance(uint16_t rayX,
                                       uint16_t rayY,
                                       uint16_t rayA,
                                       int16_t *deltaX,
                                       int16_t *deltaY,
                                       uint8_t *textureNo,
                                       uint8_t *textureX)
{
    int8_t tileStepX;
    int8_t tileStepY;
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
                if (game->IsWall(tileX, tileY)) {
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
                if (game->IsWall(tileX, tileY)) {
                    goto VerticalHit;
                }
            }
            break;
        }
    } else {
        int16_t stepX;
        int16_t stepY;

        switch (quarter) {
        case 0:
        case 1:
            tileStepX = 1;
            interceptY += MulTan(offsetX, true, quarter, angle, g_cotan);
            interceptX -= 256;
            stepX = AbsTan(quarter, angle, g_tan);
            break;
        case 2:
        case 3:
            tileStepX = -1;
            interceptY -= MulTan(offsetX, false, quarter, angle, g_cotan);
            stepX = -AbsTan(quarter, angle, g_tan);
            break;
        }

        switch (quarter) {
        case 0:
        case 3:
            tileStepY = 1;
            interceptX += MulTan(offsetY, true, quarter, angle, g_tan);
            interceptY -= 256;
            stepY = AbsTan(quarter, angle, g_cotan);
            break;
        case 1:
        case 2:
            tileStepY = -1;
            interceptX -= MulTan(offsetY, false, quarter, angle, g_tan);
            stepY = -AbsTan(quarter, angle, g_cotan);
            break;
        }

        for (;;) {
            while ((tileStepY == 1 && (interceptY >> 8 < tileY)) ||
                   (tileStepY == -1 && (interceptY >> 8 >= tileY))) {
                tileX += tileStepX;
                if (game->IsWall(tileX, tileY)) {
                    goto VerticalHit;
                }
                interceptY += stepY;
            }
            while ((tileStepX == 1 && (interceptX >> 8 < tileX)) ||
                   (tileStepX == -1 && (interceptX >> 8 >= tileX))) {
                tileY += tileStepY;
                if (game->IsWall(tileX, tileY)) {
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

// (playerX, playerY) is 8 box coordinate bits, 8 inside coordinate bits
// (playerA) is full circle as 1024
void RayCasterFixed::Trace(uint16_t screenX,
                           uint8_t *screenY,
                           uint8_t *textureNo,
                           uint8_t *textureX,
                           uint16_t *textureY,
                           uint16_t *textureStep)
{
    uint16_t rayAngle =
        static_cast<uint16_t>(_playerA + LOOKUP16(g_deltaAngle, screenX));

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
    CalculateDistance(_playerX, _playerY, rayAngle, &deltaX, &deltaY, textureNo,
                      textureX);

    // distance = deltaY * cos(playerA) + deltaX * sin(playerA)
    int16_t distance = 0;
    if (_playerA == 0) {
        distance += deltaY;
    } else if (_playerA == 512) {
        distance -= deltaY;
    } else
        switch (_viewQuarter) {
        case 0:
            distance += MulS(LOOKUP8(g_cos, _viewAngle), deltaY);
            break;
        case 1:
            distance -= MulS(LOOKUP8(g_cos, INVERT(_viewAngle)), deltaY);
            break;
        case 2:
            distance -= MulS(LOOKUP8(g_cos, _viewAngle), deltaY);
            break;
        case 3:
            distance += MulS(LOOKUP8(g_cos, INVERT(_viewAngle)), deltaY);
            break;
        }

    if (_playerA == 256) {
        distance += deltaX;
    } else if (_playerA == 768) {
        distance -= deltaX;
    } else
        switch (_viewQuarter) {
        case 0:
            distance += MulS(LOOKUP8(g_sin, _viewAngle), deltaX);
            break;
        case 1:
            distance += MulS(LOOKUP8(g_sin, INVERT(_viewAngle)), deltaX);
            break;
        case 2:
            distance -= MulS(LOOKUP8(g_sin, _viewAngle), deltaX);
            break;
        case 3:
            distance -= MulS(LOOKUP8(g_sin, INVERT(_viewAngle)), deltaX);
            break;
        }
    if (distance >= MIN_DIST) {
        *textureY = 0;
        LookupHeight((distance - MIN_DIST) >> 2, screenY, textureStep);
    } else {
        *screenY = SCREEN_HEIGHT >> 1;
        *textureY = LOOKUP16(g_overflowOffset, distance);
        *textureStep = LOOKUP16(g_overflowStep, distance);
    }
}

void RayCasterFixed::Start(uint16_t playerX, uint16_t playerY, int16_t playerA)
{
    _viewQuarter = playerA >> 8;
    _viewAngle = playerA % 256;
    _playerX = playerX;
    _playerY = playerY;
    _playerA = playerA;
}

RayCasterFixed::RayCasterFixed(Game *game) : RayCaster(game)
{
}

RayCasterFixed::~RayCasterFixed()
{
}

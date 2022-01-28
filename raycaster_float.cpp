// floating-point implementation for testing/comparison

#include "raycaster_float.h"
#include <math.h>

bool RayCasterFloat::IsWall(float rayX, float rayY)
{
    float mapX = 0;
    float mapY = 0;
    modff(rayX, &mapX);
    modff(rayY, &mapY);
    int tileX = static_cast<int>(mapX);
    int tileY = static_cast<int>(mapY);

    if (tileX < 0 || tileY < 0 || tileX >= MAP_X - 1 || tileY >= MAP_Y - 1) {
        return true;
    }
    return g_map[(tileX >> 3) + (tileY << (MAP_XS - 3))] &
           (1 << (8 - (tileX & 0x7)));
}

float RayCasterFloat::Distance(float playerX,
                               float playerY,
                               float rayA,
                               float *hitOffset,
                               int *hitDirection)
{
    while (rayA < 0) {
        rayA += 2.0f * M_PI;
    }
    while (rayA >= 2.0f * M_PI) {
        rayA -= 2.0f * M_PI;
    }

    int tileStepX = 1;
    int tileStepY = 1;
    float tileX = 0;
    float tileY = 0;
    if (rayA > M_PI) {
        tileStepX = -1;
    }
    if (rayA > M_PI_2 && rayA < 3 * M_PI_2) {
        tileStepY = -1;
    }

    float rayX = playerX;
    float rayY = playerY;
    float offsetX = modff(rayX, &tileX);
    float offsetY = modff(rayY, &tileY);

    float startDeltaX, startDeltaY;
    if (rayA <= M_PI_2) {
        startDeltaX = (1 - offsetY) * tan(rayA);
        startDeltaY = (1 - offsetX) / tan(rayA);
    } else if (rayA <= M_PI) {
        if (offsetY == 0) {
            startDeltaX = (1) * fabs(tan(rayA));
        } else {
            startDeltaX = (offsetY) *fabs(tan(rayA));
        }
        startDeltaY = -(1 - offsetX) / fabs(tan(rayA));
    } else if (rayA < 3 * M_PI_2) {
        if (offsetY == 0) {
            startDeltaX = -(1) * fabs(tan(rayA));
        } else {
            startDeltaX = -(offsetY) *fabs(tan(rayA));
        }
        if (offsetX == 0) {
            startDeltaY = -(1) / fabs(tan(rayA));
        } else {
            startDeltaY = -(offsetX) / fabs(tan(rayA));
        }
    } else {
        startDeltaX = -(1 - offsetY) * fabs(tan(rayA));
        if (offsetX == 0) {
            startDeltaY = (1) / fabs(tan(rayA));
        } else {
            startDeltaY = (offsetX) / fabs(tan(rayA));
        }
    }

    float interceptX = rayX + startDeltaX;
    float interceptY = rayY + startDeltaY;
    float stepX = fabs(tan(rayA)) * tileStepX;
    float stepY = fabs(1 / tan(rayA)) * tileStepY;
    bool verticalHit = false;
    bool horizontalHit = false;
    bool somethingDone = false;

    do {
        somethingDone = false;
        while (((tileStepY == 1 && (interceptY <= tileY + 1)) ||
                (tileStepY == -1 && (interceptY >= tileY)))) {
            somethingDone = true;
            tileX += tileStepX;
            if (IsWall(tileX, interceptY)) {
                verticalHit = true;
                rayX = tileX + (tileStepX == -1 ? 1 : 0);
                rayY = interceptY;
                *hitOffset = interceptY;
                *hitDirection = true;
                break;
            }
            interceptY += stepY;
        }
        while (!verticalHit && ((tileStepX == 1 && (interceptX <= tileX + 1)) ||
                                (tileStepX == -1 && (interceptX >= tileX)))) {
            somethingDone = true;
            tileY += tileStepY;
            if (IsWall(interceptX, tileY)) {
                horizontalHit = true;
                rayX = interceptX;
                *hitOffset = interceptX;
                *hitDirection = 0;
                rayY = tileY + (tileStepY == -1 ? 1 : 0);
                break;
            }
            interceptX += stepX;
        }
    } while ((!horizontalHit && !verticalHit) && somethingDone);

    if (!somethingDone) {
        return 0;
    }

    float deltaX = rayX - playerX;
    float deltaY = rayY - playerY;
    return sqrt(deltaX * deltaX + deltaY * deltaY);
}

void RayCasterFloat::Trace(uint16_t screenX,
                           uint8_t *screenY,
                           uint8_t *textureNo,
                           uint8_t *textureX,
                           uint16_t *textureY,
                           uint16_t *textureStep)
{
    float hitOffset;
    int hitDirection;
    float deltaAngle = atanf(((int16_t) screenX - SCREEN_WIDTH / 2.0f) /
                             (SCREEN_WIDTH / 2.0f) * M_PI / 4);
    float lineDistance = Distance(_playerX, _playerY, _playerA + deltaAngle,
                                  &hitOffset, &hitDirection);
    float distance = lineDistance * cos(deltaAngle);
    float dum;
    *textureX = (uint8_t)(256.0f * modff(hitOffset, &dum));
    *textureNo = hitDirection;
    *textureY = 0;
    *textureStep = 0;
    if (distance > 0) {
        *screenY = INV_FACTOR / distance;
        auto txs = (*screenY * 2.0f);
        if (txs != 0) {
            *textureStep = (256 / txs) * 256;
            if (txs > SCREEN_HEIGHT) {
                auto wallHeight = (txs - SCREEN_HEIGHT) / 2;
                *textureY = wallHeight * (256 / txs) * 256;
            }
        }
    } else {
        *screenY = 0;
    }
}

void RayCasterFloat::Start(uint16_t playerX, uint16_t playerY, int16_t playerA)
{
    _playerX = (playerX / 1024.0f) * 4.0f;
    _playerY = (playerY / 1024.0f) * 4.0f;
    _playerA = (playerA / 1024.0f) * 2.0f * M_PI;
}

RayCasterFloat::RayCasterFloat() : RayCaster() {}

RayCasterFloat::~RayCasterFloat() {}

// DDA + camera-plane floating-point raycaster
//
// Clean DDA implementation with camera-plane FOV representation.
// Zero dependency on precomputed tables. Perpendicular distance
// eliminates fisheye without cos(deltaAngle) correction.
//
// Coordinate system and camera geometry:
//
//            North (512, -Y)
//                 ▲
//                 │
//   West (768) ◄──┼──► East (256, +X)
//                 │
//                 ▼
//            South (0, +Y)
//
//   playerA increases: South(0) → East(256) → North(512) → West(768)
//                    = counter-clockwise in world coordinates
//
//   dir   = (sin(a), cos(a))       — unit direction vector
//   plane = (cos(a), -sin(a)) * K  — camera plane, ⊥ to dir
//
//        plane
//   ◄─── ● ───►   screenX=0 is left, screenX=W-1 is right
//        │
//        │ dir
//        ▼
//
// The 3D view is left-right mirrored (standard raycaster convention).
// Camera plane orientation preserves this: right-world renders on left-screen.

#include "raycaster_float.h"
#include <math.h>
#include <stdlib.h>

// Camera plane magnitude: FOV = 2 * atan(PLANE_MAG) ~ 76 degrees.
// Matches the original deltaAngle formula: atanf(t * pi/4).
#define PLANE_MAG ((float) M_PI / 4.0f)

// Reciprocal cap for near-zero ray direction components (avoids infinity)
#define RECIP_CAP 1e30f

typedef struct {
    float posX;
    float posY;
    float dirX;
    float dirY;
    float planeX;
    float planeY;
} RayCasterFloat;

static void RayCasterFloatTrace(RayCaster *rayCaster,
                                uint16_t screenX,
                                uint8_t *screenY,
                                uint8_t *textureNo,
                                uint8_t *textureX,
                                uint16_t *textureY,
                                uint16_t *textureStep);
static void RayCasterFloatStart(RayCaster *rayCaster,
                                uint16_t playerX,
                                uint16_t playerY,
                                int16_t playerA);
static void RayCasterFloatDestruct(RayCaster *rayCaster);

RayCaster *RayCasterFloatConstruct(void)
{
    RayCaster *rayCaster = RayCasterConstruct();
    if (!rayCaster)
        return NULL;
    RayCasterFloat *rayCasterFloat = malloc(sizeof(RayCasterFloat));
    if (!rayCasterFloat) {
        rayCaster->Destruct(rayCaster);
        return NULL;
    }
    rayCaster->derived = rayCasterFloat;

    rayCaster->Start = RayCasterFloatStart;
    rayCaster->Trace = RayCasterFloatTrace;
    rayCaster->Destruct = RayCasterFloatDestruct;

    return rayCaster;
}

static void RayCasterFloatTrace(RayCaster *rayCaster,
                                uint16_t screenX,
                                uint8_t *screenY,
                                uint8_t *textureNo,
                                uint8_t *textureX,
                                uint16_t *textureY,
                                uint16_t *textureStep)
{
    RayCasterFloat *self = rayCaster->derived;

    // Camera-plane ray generation (no trig per ray)
    float cameraX = 2.0f * screenX / (float) SCREEN_WIDTH - 1.0f;
    float rayDirX = self->dirX + self->planeX * cameraX;
    float rayDirY = self->dirY + self->planeY * cameraX;

    int mapX = (int) self->posX;
    int mapY = (int) self->posY;

    // Distance ray must travel to cross one X or Y grid line
    float deltaDX = fabsf(rayDirX) < 1e-30f ? RECIP_CAP : fabsf(1.0f / rayDirX);
    float deltaDY = fabsf(rayDirY) < 1e-30f ? RECIP_CAP : fabsf(1.0f / rayDirY);

    // Step direction and initial side distances
    int stepX, stepY;
    float sideDX, sideDY;

    if (rayDirX < 0) {
        stepX = -1;
        sideDX = (self->posX - mapX) * deltaDX;
    } else {
        stepX = 1;
        sideDX = (mapX + 1.0f - self->posX) * deltaDX;
    }
    if (rayDirY < 0) {
        stepY = -1;
        sideDY = (self->posY - mapY) * deltaDY;
    } else {
        stepY = 1;
        sideDY = (mapY + 1.0f - self->posY) * deltaDY;
    }

    // DDA: single loop, no trig inside
    int side;
    int hit = 0;
    while (!hit) {
        if (sideDX < sideDY) {
            sideDX += deltaDX;
            mapX += stepX;
            side = 0;
        } else {
            sideDY += deltaDY;
            mapY += stepY;
            side = 1;
        }
        if (mapX < 0 || mapX >= MAP_X - 1 || mapY < 0 || mapY >= MAP_Y - 1)
            hit = 1;
        else if (MapIsWall((uint8_t) mapX, (uint8_t) mapY))
            hit = 1;
    }

    // Perpendicular distance (fisheye-free, no cos correction needed).
    // Clamp to small positive value: when the player sits exactly on a grid
    // boundary the distance is 0, but the wall should fill the screen.
    float perpWallDist;
    if (side == 0)
        perpWallDist = (mapX - self->posX + (1 - stepX) / 2.0f) / rayDirX;
    else
        perpWallDist = (mapY - self->posY + (1 - stepY) / 2.0f) / rayDirY;
    if (perpWallDist < 1e-4f)
        perpWallDist = 1e-4f;

    // Texture X coordinate from hit position
    float wallX;
    if (side == 0)
        wallX = self->posY + perpWallDist * rayDirY;
    else
        wallX = self->posX + perpWallDist * rayDirX;
    wallX -= floorf(wallX);

    *textureX = (uint8_t) (wallX * 256.0f);
    // side 0 = crossed X boundary = vertical wall face (darkened by renderer)
    *textureNo = (side == 0) ? 1 : 0;

    // Wall height from perpendicular distance
    float tmp = INV_FACTOR / perpWallDist;
    float txs = tmp * 2.0f;
    *textureY = 0;
    if (txs > SCREEN_HEIGHT) {
        float wallHeight = (txs - SCREEN_HEIGHT) / 2.0f;
        *textureY = (uint16_t) (wallHeight * (256.0f / txs) * 256.0f);
        *screenY = HORIZON_HEIGHT;
    } else {
        *screenY = (uint8_t) tmp;
    }
    *textureStep = (txs > 0) ? (uint16_t) ((256.0f / txs) * 256.0f) : 0;
}

static void RayCasterFloatStart(RayCaster *rayCaster,
                                uint16_t playerX,
                                uint16_t playerY,
                                int16_t playerA)
{
    RayCasterFloat *self = rayCaster->derived;

    self->posX = playerX / 256.0f;
    self->posY = playerY / 256.0f;

    float angle = playerA / 1024.0f * 2.0f * (float) M_PI;
    self->dirX = sinf(angle);
    self->dirY = cosf(angle);

    // Camera plane perpendicular to direction, derived from dir vector.
    // Plane orientation (dirY, -dirX) preserves the mirrored 3D view.
    self->planeX = self->dirY * PLANE_MAG;
    self->planeY = -self->dirX * PLANE_MAG;
}

static void RayCasterFloatDestruct(RayCaster *rayCaster)
{
    free(rayCaster->derived);
    RayCasterDestruct(rayCaster);
}

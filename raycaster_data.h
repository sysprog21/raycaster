#pragma once

#include <stdbool.h>

#include "raycaster.h"

extern const uint8_t LOOKUP_TBL g_map[128];

extern const uint32_t LOOKUP_TBL g_texture32[4096];

extern const uint8_t LOOKUP_TBL g_font[128][16];

/* Compiled-in bitfield wall check (always available) */
static inline bool MapIsWallBuiltin(uint8_t tileX, uint8_t tileY)
{
    if (tileX >= MAP_X - 1 || tileY >= MAP_Y - 1) {
        return true;
    }
    return LOOKUP8(g_map, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
           (1 << (8 - (tileX & 0x7)));
}

#ifdef RAYCASTER_HAS_RUNTIME_MAP
/* Runtime map state: extern globals for zero-overhead inline access.
 * Set once by MapRuntimeInstall(), read on every DDA step.
 */
extern bool g_mapRuntimeActive;
extern uint8_t g_mapRuntimeW;
extern uint8_t g_mapRuntimeH;
extern bool MapRuntimeIsWall(uint8_t tileX, uint8_t tileY);

static inline bool MapIsWall(uint8_t tileX, uint8_t tileY)
{
    if (g_mapRuntimeActive)
        return MapRuntimeIsWall(tileX, tileY);
    return MapIsWallBuiltin(tileX, tileY);
}

/* Effective map dimensions: runtime if loaded, compiled-in otherwise */
static inline uint8_t MapWidth(void)
{
    return g_mapRuntimeActive ? g_mapRuntimeW : MAP_X;
}

static inline uint8_t MapHeight(void)
{
    return g_mapRuntimeActive ? g_mapRuntimeH : MAP_Y;
}
#else
/* Baremetal: direct bitfield check, no runtime map overhead */
static inline bool MapIsWall(uint8_t tileX, uint8_t tileY)
{
    return MapIsWallBuiltin(tileX, tileY);
}

static inline uint8_t MapWidth(void)
{
    return MAP_X;
}

static inline uint8_t MapHeight(void)
{
    return MAP_Y;
}
#endif

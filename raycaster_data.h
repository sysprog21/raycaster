#pragma once

#include <stdbool.h>

#include "raycaster.h"

extern const uint8_t LOOKUP_TBL g_map[128];

extern const uint32_t LOOKUP_TBL g_texture32[4096];

extern const uint8_t LOOKUP_TBL g_font[128][16];

inline bool MapIsWall(uint8_t tileX, uint8_t tileY)
{
    if (tileX >= MAP_X - 1 || tileY >= MAP_Y - 1) {
        return true;
    }
    return LOOKUP8(g_map, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
           (1 << (8 - (tileX & 0x7)));
}
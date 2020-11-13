#pragma once

#include <cstdint>
#include "raycaster_data.h"

#define MAP_X (uint8_t) 32
#define MAP_XS (uint8_t) 5
#define MAP_Y (uint8_t) 32

class Map
{
public:
    Map(const uint8_t *mapData) : mapData(mapData) {}
    inline bool IsWall(uint8_t tileX, uint8_t tileY) const
    {
        if (tileX > MAP_X - 1 || tileY > MAP_Y - 1) {
            return true;
        }
        return LOOKUP8(mapData, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
               (1 << (8 - (tileX & 0x7)));
    }

    ~Map(){};

private:
    const uint8_t *mapData;
};
#include "map.h"
#include "raycaster_data.h"

bool Map::IsWall(uint8_t tileX, uint8_t tileY) const
{
    if (tileX > MAP_X - 1 || tileY > MAP_Y - 1) {
        return true;
    }
    return LOOKUP8(mapData, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
           (1 << (8 - (tileX & 0x7)));
}
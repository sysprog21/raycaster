#pragma once

#include <cstdint>

#define MAP_X (uint8_t) 32
#define MAP_XS (uint8_t) 5
#define MAP_Y (uint8_t) 32

class Map
{
public:
    Map(const uint8_t *mapData) : mapData(mapData) {}
    bool IsWall(uint8_t tileX, uint8_t tileY) const;

    ~Map(){};

private:
    const uint8_t *mapData;
};
#pragma once

#include <cstdint>

#define MAP_X 32
#define MAP_XS 5
#define MAP_Y 32

class Map
{
public:
    Map(const uint8_t *mapData) : mapData(mapData) {}
    bool IsWall(uint8_t tileX, uint8_t tileY) const;

    ~Map(){};

private:
    const uint8_t *mapData;
};
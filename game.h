#pragma once

#include <stdint.h>
#include "map.h"

class Game
{
public:
    void Move(int m, int r, float seconds);

    float playerX, playerY, playerA;
    Map map;

    Game(const uint8_t *map);
    ~Game();
};

#pragma once

#include <stdint.h>
#include "map.h"

class Game
{
public:
    void Move(int m, int r, float seconds);

    float playerX, playerY, playerA;
    Map *map;

    Game(Map *map);
    ~Game();
};

#pragma once

#include <stdint.h>

class Game
{
public:
    void Move(int m, int r, float seconds);
    bool IsWall(uint8_t tileX, uint8_t tileY);

    float playerX, playerY, playerA;

    Game();
    ~Game();
};

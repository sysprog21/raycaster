#pragma once

#include <stdint.h>

class Game
{
public:
    void Move(int m, int r, float seconds);

    float playerX, playerY, playerA;

    Game();
    ~Game();
};

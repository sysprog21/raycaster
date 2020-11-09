#pragma once

#include <stdint.h>

// treat player as a circle for collision test
// should be smaller than 0.5
#define PLAYER_RADIUS 0.2f

class Game
{
public:
    void Move(int m, int r, float seconds);
    bool IsWall(uint8_t tileX, uint8_t tileY);

    float playerX, playerY, playerA;

    Game();
    ~Game();

private:
    void CollideWithMap();
};

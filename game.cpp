#include <cmath>
#include <cstdlib>

#include "game.h"
#include "raycaster.h"
#include "raycaster_data.h"

void Game::Move(int m, int r, float seconds)
{
    playerA += 0.05f * r * seconds * 25.0f;
    playerX += 0.5f * m * sin(playerA) * seconds * 5.0f;
    playerY += 0.5f * m * cos(playerA) * seconds * 5.0f;

    while (playerA < 0) {
        playerA += 2.0f * M_PI;
    }
    while (playerA >= 2.0f * M_PI) {
        playerA -= 2.0f * M_PI;
    }

    if (playerX < 1) {
        playerX = 1.01f;
    } else if (playerX > MAP_X - 2) {
        playerX = MAP_X - 2 - 0.01f;
    }
    if (playerY < 1) {
        playerY = 1.01f;
    } else if (playerY > MAP_Y - 2) {
        playerY = MAP_Y - 2 - 0.01f;
    }
}

Game::Game()
{
    playerX = 23.03f;
    playerY = 6.8f;
    playerA = 5.25f;
}

bool Game::IsWall(uint8_t tileX, uint8_t tileY)
{
    if (tileX > MAP_X - 1 || tileY > MAP_Y - 1) {
        return true;
    }
    return LOOKUP8(g_map, (tileX >> 3) + (tileY << (MAP_XS - 3))) &
           (1 << (8 - (tileX & 0x7)));
}

Game::~Game()
{
}

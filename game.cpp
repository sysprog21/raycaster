#include <cmath>
#include <cstdlib>
#include <cassert>

#include "game.h"
#include "raycaster.h"
#include "raycaster_data.h"

static inline float distance(float x1, float y1, float x2, float y2)
{
    x1 -= x2;
    y1 -= y2;
    return sqrtf(x1 * x1 + y1 * y1);
}

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

    CheckWallCollisions();
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

void Game::CheckWallCollisions()
{
    float offsetX, offsetY, mapX, mapY;
    bool collided;
    offsetX = modff(playerX, &mapX);
    offsetY = modff(playerY, &mapY);

    static_assert(PLAYER_RADIUS < 0.5f,
                  "Player size should be smaller than a block (1x1).");

    // check collisions with four neighboring wall planes
    collided = false;
    if ((1 - offsetX) < PLAYER_RADIUS && IsWall(mapX + 1, mapY)) {
        playerX = mapX + 1 - PLAYER_RADIUS;
        collided = true;
    } else if (offsetX < PLAYER_RADIUS && IsWall(mapX - 1, mapY)) {
        playerX = mapX + PLAYER_RADIUS;
        collided = true;
    }
    if ((1 - offsetY) < PLAYER_RADIUS && IsWall(mapX, mapY + 1)) {
        playerY = mapY + 1 - PLAYER_RADIUS;
        collided = true;
    } else if (offsetY < PLAYER_RADIUS && IsWall(mapX, mapY - 1)) {
        playerY = mapY + PLAYER_RADIUS;
        collided = true;
    }

    if (collided)
        return;

    // check collisions with four corners
    // we only have to do this if the player haven't collided with any planes
    for (int i = -1; i <= 1; i += 2) {
        for (int j = -1; j <= 1; j += 2) {
            float cornerDistance =
                distance(playerX, playerY, mapX + (i > 0), mapY + (j > 0)) -
                PLAYER_RADIUS;
            if (cornerDistance < 0 && IsWall(mapX + i, mapY + j)) {
                float cornerA = atan2f((mapY + (j > 0) - playerY),
                                       (mapX + (i > 0) - playerX));
                playerX += cornerDistance * cosf(cornerA);
                playerY += cornerDistance * sinf(cornerA);
                return;
            }
        }
    }
}

Game::~Game()
{
}

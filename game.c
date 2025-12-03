#include <math.h>
#include <stdlib.h>

#include "game.h"
#include "raycaster.h"
#include "raycaster_data.h"
#include "raycaster_tables.h"

Game GameConstruct(void)
{
    Game game;
    game.playerX = 5895;  // (uint16_t) (23.03f * 256)
    game.playerY = 1740;  // (uint16_t) (6.8f * 256)
    game.playerA = 320;   // (uint16_t) (5.25f * 256) - 1024
    return game;
}

void GameMove(Game *game, int m, int r, uint16_t seconds)
{
    game->playerA += r * UMULT(320, seconds);

    while (game->playerA < 0) {
        game->playerA += 1024;
    }
    while (game->playerA >= 1024) {
        game->playerA -= 1024;
    }

    uint8_t angle = game->playerA % 256;
    int16_t sine = 0, cosine = 0;
    switch (game->playerA >> 8) {
    case 0:
        sine += LOOKUP8(g_sin, angle);
        cosine += LOOKUP8(g_cos, angle);
        break;
    case 1:
        sine += LOOKUP8(g_sin, INVERT(angle));
        cosine -= LOOKUP8(g_cos, INVERT(angle));
        break;
    case 2:
        sine -= LOOKUP8(g_sin, angle);
        cosine -= LOOKUP8(g_cos, angle);
        break;
    case 3:
        sine -= LOOKUP8(g_sin, INVERT(angle));
        cosine += LOOKUP8(g_cos, INVERT(angle));
        break;
    }
    /* Use int32_t to prevent overflow: UMULT can be up to 65535, *5>>1 ~163k */
    int32_t dx = ((int32_t) m * (sine > 0 ? 1 : -1) *
                  UMULT(sine > 0 ? sine : -sine, seconds) * 5) >>
                 1;
    int32_t dy = ((int32_t) m * (cosine > 0 ? 1 : -1) *
                  UMULT(cosine > 0 ? cosine : -cosine, seconds) * 5) >>
                 1;

    /* Boundary constants */
    const int32_t minBound = 256 + PLAYER_RADIUS;
    const int32_t maxBoundX = ((MAP_X - 2) << 8) - PLAYER_RADIUS;
    const int32_t maxBoundY = ((MAP_Y - 2) << 8) - PLAYER_RADIUS;

    /* Check X movement with player radius (use int32_t to prevent underflow) */
    int32_t newX = (int32_t) game->playerX + dx;
    if (newX < minBound)
        newX = minBound;
    else if (newX > maxBoundX)
        newX = maxBoundX;

    /* Check Y movement with player radius (use int32_t to prevent underflow) */
    int32_t newY = (int32_t) game->playerY + dy;
    if (newY < minBound)
        newY = minBound;
    else if (newY > maxBoundY)
        newY = maxBoundY;

    /* Diagonal corner check: prevent tunneling through wall corners */
    int canMoveX = 1, canMoveY = 1;
    if (dx != 0 && dy != 0) {
        int32_t diagX =
            (dx >= 0) ? (newX + PLAYER_RADIUS) : (newX - PLAYER_RADIUS);
        int32_t diagY =
            (dy >= 0) ? (newY + PLAYER_RADIUS) : (newY - PLAYER_RADIUS);
        int diagTileX = diagX >> 8;
        int diagTileY = diagY >> 8;

        if (MapIsWall(diagTileX, diagTileY)) {
            /* Corner is blocked - check which axis is the actual obstacle */
            int32_t curXedge = (dx >= 0) ? (game->playerX + PLAYER_RADIUS)
                                         : (game->playerX - PLAYER_RADIUS);
            int32_t curYedge = (dy >= 0) ? (game->playerY + PLAYER_RADIUS)
                                         : (game->playerY - PLAYER_RADIUS);
            int curTileX = curXedge >> 8;
            int curTileY = curYedge >> 8;

            /* If moving X with current Y hits wall, block X */
            if (MapIsWall(diagTileX, curTileY))
                canMoveX = 0;
            /* If moving Y with current X hits wall, block Y */
            if (MapIsWall(curTileX, diagTileY))
                canMoveY = 0;
            /* If neither axis alone is blocked, slide along closer surface */
            if (canMoveX && canMoveY) {
                /* Compare clearance to wall on each axis */
                int32_t clearX = (dx >= 0) ? ((diagTileX << 8) - newX)
                                           : (newX - ((diagTileX + 1) << 8));
                int32_t clearY = (dy >= 0) ? ((diagTileY << 8) - newY)
                                           : (newY - ((diagTileY + 1) << 8));
                if (clearX <= clearY)
                    canMoveX = 0;
                else
                    canMoveY = 0;
            }
        }
    }

    if (dx != 0 && canMoveX) {
        int32_t checkX =
            (dx >= 0) ? (newX + PLAYER_RADIUS) : (newX - PLAYER_RADIUS);
        int32_t checkYhi = (int32_t) game->playerY + PLAYER_RADIUS;
        int32_t checkYlo = (int32_t) game->playerY - PLAYER_RADIUS;
        if (checkYlo < 0)
            checkYlo = 0;
        if (!MapIsWall(checkX >> 8, checkYhi >> 8) &&
            !MapIsWall(checkX >> 8, checkYlo >> 8)) {
            game->playerX = (uint16_t) newX;
        }
    }

    if (dy != 0 && canMoveY) {
        int32_t checkY =
            (dy >= 0) ? (newY + PLAYER_RADIUS) : (newY - PLAYER_RADIUS);
        int32_t checkXhi = (int32_t) game->playerX + PLAYER_RADIUS;
        int32_t checkXlo = (int32_t) game->playerX - PLAYER_RADIUS;
        if (checkXlo < 0)
            checkXlo = 0;
        if (!MapIsWall(checkXhi >> 8, checkY >> 8) &&
            !MapIsWall(checkXlo >> 8, checkY >> 8)) {
            game->playerY = (uint16_t) newY;
        }
    }

    /* Final boundary clamp (safety net) */
    if (game->playerX < minBound)
        game->playerX = minBound;
    else if (game->playerX > maxBoundX)
        game->playerX = maxBoundX;
    if (game->playerY < minBound)
        game->playerY = minBound;
    else if (game->playerY > maxBoundY)
        game->playerY = maxBoundY;
}
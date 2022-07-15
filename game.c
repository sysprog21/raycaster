#include <math.h>
#include <stdlib.h>

#include "game.h"
#include "raycaster.h"

Game GameConstruct(void)
{
    Game game;
    game.playerX = 23.03f;
    game.playerY = 6.8f;
    game.playerA = 5.25f;
    return game;
}

void GameMove(Game *game, int m, int r, float seconds)
{
    game->playerA += 0.05f * r * seconds * 25.0f;
    game->playerX += 0.5f * m * sin(game->playerA) * seconds * 5.0f;
    game->playerY += 0.5f * m * cos(game->playerA) * seconds * 5.0f;

    while (game->playerA < 0) {
        game->playerA += 2.0f * M_PI;
    }
    while (game->playerA >= 2.0f * M_PI) {
        game->playerA -= 2.0f * M_PI;
    }

    if (game->playerX < 1) {
        game->playerX = 1.01f;
    } else if (game->playerX > MAP_X - 2) {
        game->playerX = MAP_X - 2 - 0.01f;
    }
    if (game->playerY < 1) {
        game->playerY = 1.01f;
    } else if (game->playerY > MAP_Y - 2) {
        game->playerY = MAP_Y - 2 - 0.01f;
    }
}
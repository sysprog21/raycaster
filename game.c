#include <math.h>
#include <stdlib.h>

#include "game.h"
#include "raycaster.h"
#include "raycaster_tables.h"

Game GameConstruct(void)
{
    Game game;
    game.playerX = 5895;  // 23.03f * 256.0f
    game.playerY = 1740;  // 6.8f * 256.0f
    game.playerA = 1344;  // 5.25f * 256.0f
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
    game->playerX += (m * (sine > 0 ? 1 : -1) *
                      UMULT(sine > 0 ? sine : -sine, seconds) * 5) >>
                     1;
    game->playerY += (m * (cosine > 0 ? 1 : -1) *
                      UMULT(cosine > 0 ? cosine : -cosine, seconds) * 5) >>
                     1;

    if (game->playerX < 256) {
        game->playerX = 258;
    } else if (game->playerX > (MAP_X - 2) << 8) {
        game->playerX = ((MAP_X - 2) << 8) - 2;
    }
    if (game->playerY < 256) {
        game->playerY = 258;
    } else if (game->playerY > (MAP_Y - 2) << 8) {
        game->playerY = ((MAP_Y - 2) << 8) - 2;
    }
}
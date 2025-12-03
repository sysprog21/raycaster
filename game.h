#pragma once

#include <stdint.h>

/* Player collision radius in 8.8 fixed-point (0.2 * 256 = 51) */
#define PLAYER_RADIUS 51

typedef struct {
    uint16_t playerX, playerY;
    int16_t playerA;
} Game;

Game GameConstruct(void);

void GameMove(Game *game, int m, int r, uint16_t seconds);

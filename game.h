#pragma once

#include <stdint.h>

/* Player collision radius in 8.8 fixed-point (0.2 * 256 = 51) */
#define PLAYER_RADIUS 51

typedef struct {
    uint16_t playerX, playerY;
    int16_t playerA;
} Game;

/* Default spawn (compiled-in map) */
Game GameConstruct(void);

/* Spawn at specific position (from parsed .ray file) */
Game GameConstructAt(uint16_t x, uint16_t y, int16_t a);

void GameMove(Game *game, int m, int r, uint16_t seconds);

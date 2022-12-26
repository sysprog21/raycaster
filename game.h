#pragma once

#include <stdint.h>

typedef struct {
    uint16_t playerX, playerY;
    int16_t playerA;
} Game;

Game GameConstruct(void);

void GameMove(Game *game, int m, int r, uint16_t seconds);

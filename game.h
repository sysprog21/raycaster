#pragma once

#include <stdint.h>

typedef struct Game {
    float playerX, playerY, playerA;
} Game;

Game GameConstruct(void);

void GameMove(Game *game, int m, int r, float seconds);
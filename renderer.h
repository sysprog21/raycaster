#pragma once

#include "game.h"
#include "raycaster.h"

typedef struct {
    RayCaster *rc;
} Renderer;

Renderer RendererConstruct(RayCaster *rc);

void RendererDestruct(Renderer *renderer);

void RendererTraceFrame(Renderer *renderer, Game *g, uint32_t *frameBuffer);

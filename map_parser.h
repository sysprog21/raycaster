#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Maximum map dimensions for .ray files */
#define MAP_MAX_X 64
#define MAP_MAX_Y 64

/* Maximum line length in .ray files */
#define MAP_LINE_MAX 256

/* Maximum file size for .ray files (64 KB) */
#define MAP_FILE_MAX (64 * 1024)

/* Maximum texture path length */
#define MAP_PATH_MAX 128

typedef struct {
    /* Map grid: 0 = floor, 1 = wall. Row-major: grid[y * width + x] */
    uint8_t grid[MAP_MAX_Y * MAP_MAX_X];
    uint8_t width;
    uint8_t height;

    /* Player spawn in 8.8 fixed-point (center of spawn tile + 128) */
    uint16_t spawnX;
    uint16_t spawnY;
    int16_t spawnA; /* 0-1023 angle system */

    /* Texture paths (parsed but not loaded until NSWE textures are implemented)
     */
    char texN[MAP_PATH_MAX];
    char texS[MAP_PATH_MAX];
    char texW[MAP_PATH_MAX];
    char texE[MAP_PATH_MAX];

    /* Floor and ceiling colors (RGB, 0-255 each) */
    uint8_t floorR, floorG, floorB;
    uint8_t ceilR, ceilG, ceilB;
} MapConfig;

/* Parse a .ray map file. Returns true on success, false on error.
 * Error messages are printed to stderr.
 */
bool MapConfigLoad(const char *filename, MapConfig *config);

/* Install a parsed map as the active runtime map.
 * After this call, MapIsWall() uses the runtime grid instead of compiled-in
 * data.
 */
void MapRuntimeInstall(const MapConfig *config);

/* Runtime map dimensions. Only valid when g_mapRuntimeActive is true. */
uint8_t MapRuntimeWidth(void);
uint8_t MapRuntimeHeight(void);

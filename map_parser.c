#include "map_parser.h"
#include "raycaster_data.h" /* cross-check extern decls of runtime map globals */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Runtime map state -- globals so the inline MapIsWall can read them
 * directly without a function call on every DDA step.
 */
bool g_mapRuntimeActive = false;
uint8_t g_mapRuntimeW = 0;
uint8_t g_mapRuntimeH = 0;
static uint8_t g_runtimeGrid[MAP_MAX_Y * MAP_MAX_X];

uint8_t MapRuntimeWidth(void)
{
    return g_mapRuntimeW;
}

uint8_t MapRuntimeHeight(void)
{
    return g_mapRuntimeH;
}

void MapRuntimeInstall(const MapConfig *config)
{
    if (config->width < 5 || config->width > MAP_MAX_X || config->height < 5 ||
        config->height > MAP_MAX_Y)
        return;
    g_mapRuntimeW = config->width;
    g_mapRuntimeH = config->height;
    memcpy(g_runtimeGrid, config->grid, (size_t) g_mapRuntimeW * g_mapRuntimeH);
    g_mapRuntimeActive = true;
}

/* Runtime wall check used by MapIsWall when a runtime map is installed.
 * Declared extern in raycaster_data.h for SDL builds.
 */
bool MapRuntimeIsWall(uint8_t tileX, uint8_t tileY)
{
    if (tileX + 1 >= g_mapRuntimeW || tileY + 1 >= g_mapRuntimeH)
        return true;
    return g_runtimeGrid[tileY * g_mapRuntimeW + tileX] != 0;
}

/* --- Parser internals --- */

static char *strip(char *s)
{
    while (*s && isspace((unsigned char) *s))
        s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char) end[-1]))
        end--;
    *end = '\0';
    return s;
}

/* Parse "R,G,B" string. Returns true on success. */
static bool parse_rgb(const char *s, uint8_t *r, uint8_t *g, uint8_t *b)
{
    int ri, gi, bi;
    char extra;
    if (sscanf(s, " %d , %d , %d %c", &ri, &gi, &bi, &extra) != 3)
        return false;
    if (ri < 0 || ri > 255 || gi < 0 || gi > 255 || bi < 0 || bi > 255)
        return false;
    *r = (uint8_t) ri;
    *g = (uint8_t) gi;
    *b = (uint8_t) bi;
    return true;
}

/* Copy texture path with length check */
static bool set_tex_path(char *dst, const char *src)
{
    const char *s = src;
    while (*s && isspace((unsigned char) *s))
        s++;
    if (strlen(s) == 0 || strlen(s) >= MAP_PATH_MAX)
        return false;
    strncpy(dst, s, MAP_PATH_MAX - 1);
    dst[MAP_PATH_MAX - 1] = '\0';
    /* Trim trailing whitespace */
    char *end = dst + strlen(dst);
    while (end > dst && isspace((unsigned char) end[-1]))
        end--;
    *end = '\0';
    return true;
}

/* Flood-fill from (x,y) marking visited cells. Returns true if any cell
 * on the map border is reachable (meaning the map is NOT enclosed).
 */
static bool flood_reaches_border(const uint8_t *grid,
                                 uint8_t w,
                                 uint8_t h,
                                 uint8_t *visited,
                                 int startX,
                                 int startY)
{
    /* BFS using a simple queue */
    typedef struct {
        uint8_t x, y;
    } Pos;
    Pos *queue = malloc(w * h * sizeof(Pos));
    if (!queue)
        return true; /* fail safe: treat as open */
    int head = 0, tail = 0;
    bool reachesBorder = false;

    visited[startY * w + startX] = 1;
    queue[tail++] = (Pos) {startX, startY};

    while (head < tail) {
        Pos cur = queue[head++];
        /* Check if this floor cell is on the border */
        if (cur.x == 0 || cur.x == w - 1 || cur.y == 0 || cur.y == h - 1) {
            reachesBorder = true;
            break;
        }
        /* 4-directional neighbors */
        const int dx[] = {-1, 1, 0, 0};
        const int dy[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; d++) {
            int nx = cur.x + dx[d];
            int ny = cur.y + dy[d];
            if (nx < 0 || nx >= w || ny < 0 || ny >= h)
                continue;
            int idx = ny * w + nx;
            if (!visited[idx] && grid[idx] == 0) {
                visited[idx] = 1;
                queue[tail++] = (Pos) {(uint8_t) nx, (uint8_t) ny};
            }
        }
    }
    free(queue);
    return reachesBorder;
}

/* Validate that all floor cells are fully enclosed by walls.
 * Flood-fill from every unvisited floor cell; if any fill reaches the
 * map border, the map has a gap.
 */
static bool validate_enclosure(const uint8_t *grid, uint8_t w, uint8_t h)
{
    uint8_t *visited = calloc(w * h, 1);
    if (!visited)
        return false;

    bool ok = true;
    for (int y = 0; y < h && ok; y++) {
        for (int x = 0; x < w && ok; x++) {
            int idx = y * w + x;
            if (grid[idx] == 0 && !visited[idx]) {
                if (flood_reaches_border(grid, w, h, visited, x, y)) {
                    fprintf(stderr,
                            "map error: floor at (%d,%d) is not enclosed by "
                            "walls\n",
                            x, y);
                    ok = false;
                }
            }
        }
    }
    free(visited);
    return ok;
}

/* Direction character to angle (0-1023 system) */
static int16_t dir_to_angle(char c)
{
    switch (c) {
    case 'N':
        return 512;
    case 'S':
        return 0;
    case 'E':
        return 256;
    case 'W':
        return 768;
    default:
        return -1;
    }
}

bool MapConfigLoad(const char *filename, MapConfig *config)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "map error: cannot open '%s'\n", filename);
        return false;
    }

    /* Check file size */
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz > MAP_FILE_MAX) {
        fprintf(stderr, "map error: file exceeds %d bytes\n", MAP_FILE_MAX);
        fclose(fp);
        return false;
    }

    memset(config, 0, sizeof(*config));

    /* Flags for required config lines */
    bool hasTexN = false, hasTexS = false, hasTexW = false, hasTexE = false;
    bool hasFloor = false, hasCeil = false;

    /* Pass 1: read config lines (before map grid) */
    char line[MAP_LINE_MAX];
    int lineNo = 0;
    long mapStart = -1;

    while (1) {
        long lineStart = ftell(fp);
        if (!fgets(line, sizeof(line), fp))
            break;
        lineNo++;
        char *s = strip(line);
        if (strlen(s) == 0)
            continue;

        /* Config lines: "KEY value" */
        if (strncmp(s, "T_N ", 4) == 0) {
            if (hasTexN) {
                fprintf(stderr, "line %d: duplicate T_N\n", lineNo);
                fclose(fp);
                return false;
            }
            if (!set_tex_path(config->texN, s + 4)) {
                fprintf(stderr, "line %d: invalid T_N path\n", lineNo);
                fclose(fp);
                return false;
            }
            hasTexN = true;
        } else if (strncmp(s, "T_S ", 4) == 0) {
            if (hasTexS) {
                fprintf(stderr, "line %d: duplicate T_S\n", lineNo);
                fclose(fp);
                return false;
            }
            if (!set_tex_path(config->texS, s + 4)) {
                fprintf(stderr, "line %d: invalid T_S path\n", lineNo);
                fclose(fp);
                return false;
            }
            hasTexS = true;
        } else if (strncmp(s, "T_W ", 4) == 0) {
            if (hasTexW) {
                fprintf(stderr, "line %d: duplicate T_W\n", lineNo);
                fclose(fp);
                return false;
            }
            if (!set_tex_path(config->texW, s + 4)) {
                fprintf(stderr, "line %d: invalid T_W path\n", lineNo);
                fclose(fp);
                return false;
            }
            hasTexW = true;
        } else if (strncmp(s, "T_E ", 4) == 0) {
            if (hasTexE) {
                fprintf(stderr, "line %d: duplicate T_E\n", lineNo);
                fclose(fp);
                return false;
            }
            if (!set_tex_path(config->texE, s + 4)) {
                fprintf(stderr, "line %d: invalid T_E path\n", lineNo);
                fclose(fp);
                return false;
            }
            hasTexE = true;
        } else if (s[0] == 'F' && s[1] == ' ') {
            if (hasFloor) {
                fprintf(stderr, "line %d: duplicate F\n", lineNo);
                fclose(fp);
                return false;
            }
            if (!parse_rgb(s + 2, &config->floorR, &config->floorG,
                           &config->floorB)) {
                fprintf(stderr, "line %d: invalid floor color (want R,G,B)\n",
                        lineNo);
                fclose(fp);
                return false;
            }
            hasFloor = true;
        } else if (s[0] == 'C' && s[1] == ' ') {
            if (hasCeil) {
                fprintf(stderr, "line %d: duplicate C\n", lineNo);
                fclose(fp);
                return false;
            }
            if (!parse_rgb(s + 2, &config->ceilR, &config->ceilG,
                           &config->ceilB)) {
                fprintf(stderr, "line %d: invalid ceiling color (want R,G,B)\n",
                        lineNo);
                fclose(fp);
                return false;
            }
            hasCeil = true;
        } else {
            /* First non-config, non-empty line is the start of the map grid */
            mapStart = lineStart;
            break;
        }
    }

    if (!hasTexN || !hasTexS || !hasTexW || !hasTexE) {
        fprintf(stderr,
                "map error: missing texture path(s) (need T_N, T_S, T_W, "
                "T_E)\n");
        fclose(fp);
        return false;
    }
    if (!hasFloor || !hasCeil) {
        fprintf(stderr, "map error: missing floor (F) or ceiling (C) color\n");
        fclose(fp);
        return false;
    }

    /* Pass 2: read map grid */
    if (mapStart < 0) {
        fprintf(stderr, "map error: no map grid found\n");
        fclose(fp);
        return false;
    }
    fseek(fp, mapStart, SEEK_SET);

    int mapW = 0, mapH = 0;
    int spawnCount = 0;
    int spawnX = 0, spawnY = 0;
    char spawnDir = '\0';

    /* Temporary storage for map lines to determine width */
    char mapLines[MAP_MAX_Y][MAP_LINE_MAX];

    while (fgets(line, sizeof(line), fp)) {
        /* Strip trailing whitespace only (preserve leading spaces as empty
         * columns) */
        char *end = line + strlen(line);
        while (end > line && isspace((unsigned char) end[-1]))
            end--;
        *end = '\0';

        /* Skip blank lines after map starts (tolerance) */
        if (strlen(line) == 0)
            continue;

        if (mapH >= MAP_MAX_Y) {
            fprintf(stderr, "map error: map exceeds %d rows\n", MAP_MAX_Y);
            fclose(fp);
            return false;
        }

        int rowLen = (int) strlen(line);
        if (rowLen > MAP_MAX_X) {
            fprintf(stderr, "map error: row %d exceeds %d columns\n", mapH + 1,
                    MAP_MAX_X);
            fclose(fp);
            return false;
        }

        strncpy(mapLines[mapH], line, MAP_LINE_MAX - 1);
        mapLines[mapH][MAP_LINE_MAX - 1] = '\0';

        if (rowLen > mapW)
            mapW = rowLen;
        mapH++;
    }
    fclose(fp);

    if (mapH < 5 || mapW < 5) {
        fprintf(stderr, "map error: map too small (minimum 5x5)\n");
        return false;
    }

    config->width = (uint8_t) mapW;
    config->height = (uint8_t) mapH;

    /* Parse grid characters */
    for (int y = 0; y < mapH; y++) {
        int rowLen = (int) strlen(mapLines[y]);
        for (int x = 0; x < mapW; x++) {
            char c = (x < rowLen) ? mapLines[y][x] : ' ';
            int idx = y * mapW + x;

            switch (c) {
            case '1':
                config->grid[idx] = 1;
                break;
            case '0':
                config->grid[idx] = 0;
                break;
            case 'N':
            case 'S':
            case 'E':
            case 'W':
                config->grid[idx] = 0; /* spawn tile is floor */
                spawnX = x;
                spawnY = y;
                spawnDir = c;
                spawnCount++;
                break;
            case ' ':
                /* Space outside the playable area -- treat as wall for
                 * enclosure purposes */
                config->grid[idx] = 1;
                break;
            default:
                fprintf(stderr,
                        "map error: invalid character '%c' at row %d col %d\n",
                        c, y + 1, x + 1);
                return false;
            }
        }
    }

    if (spawnCount == 0) {
        fprintf(stderr, "map error: no player spawn (need one N/S/E/W)\n");
        return false;
    }
    if (spawnCount > 1) {
        fprintf(stderr,
                "map error: multiple spawns found (need exactly one)\n");
        return false;
    }

    /* Convert spawn tile to 8.8 fixed-point (center of tile) */
    config->spawnX = (uint16_t) ((spawnX << 8) + 128);
    config->spawnY = (uint16_t) ((spawnY << 8) + 128);
    config->spawnA = dir_to_angle(spawnDir);

    /* Pass 3: validate enclosure */
    if (!validate_enclosure(config->grid, config->width, config->height)) {
        return false;
    }

    return true;
}

#pragma once

#include "raycaster.h"

#ifdef TABLES_320
#define HAS_TABLES

extern const uint16_t LOOKUP_TBL g_tan[256];

extern const uint16_t LOOKUP_TBL g_cotan[256];

extern const uint8_t LOOKUP_TBL g_sin[256];

extern const uint8_t LOOKUP_TBL g_cos[256];

extern const uint8_t LOOKUP_TBL g_nearHeight[256];

extern const uint8_t LOOKUP_TBL g_farHeight[256];

extern const uint16_t LOOKUP_TBL g_nearStep[256];

extern const uint16_t LOOKUP_TBL g_farStep[256];

extern const uint16_t LOOKUP_TBL g_overflowOffset[256];

extern const uint16_t LOOKUP_TBL g_overflowStep[256];

extern const uint16_t LOOKUP_TBL g_deltaAngle[SCREEN_WIDTH];


#endif

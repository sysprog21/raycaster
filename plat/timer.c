#include "timer.h"

#include "mmio.h"

#define TIMER_BASE 0x20003000
#define TIMER_CS (TIMER_BASE + 0x00)
#define TIMER_CLO (TIMER_BASE + 0x04)
#define TIMER_CHI (TIMER_BASE + 0x08)
#define TIMER_C0 (TIMER_BASE + 0x0C)
#define TIMER_C1 (TIMER_BASE + 0x10)
#define TIMER_C2 (TIMER_BASE + 0x14)
#define TIMER_C3 (TIMER_BASE + 0x18)

uint64_t timer_clock(void)
{
    return ((uint64_t) mmio_read(TIMER_CHI) << 32) |
           ((uint64_t) mmio_read(TIMER_CLO));
}
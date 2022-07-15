#ifndef MMIO_H
#define MMIO_H

#include <stdint.h>

inline void mmio_write(uint32_t reg, uint32_t data)
{
    *(volatile uint32_t *) (uintptr_t) reg = data;
}

inline uint32_t mmio_read(uint32_t reg)
{
    return *(volatile uint32_t *) (uintptr_t) reg;
}

void spin(uint32_t count);

#endif  // MMIO_H

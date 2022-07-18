#ifndef FB_H
#define FB_H

#include <stdint.h>

void *fb_create(uint32_t width, uint32_t height, uint32_t depth);

void fb_putc(uint32_t *fb,
             int width,
             int height,
             const uint8_t *font,
             int x,
             int y);

void fb_puts(uint32_t *fb,
             int width,
             int height,
             const uint8_t (*font)[16],
             const char *str,
             int x,
             int y);

#endif  // FB_H
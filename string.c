#include <string.h>

void *memcpy(void *dest, const void *src, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        ((char *) dest)[i] = ((char *) src)[i];
    }
    return dest;
}

void *memset(void *dest, int ch, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        ((char *) dest)[i] = ch;
    }
    return dest;
}
#include <string.h>

void *memcpy(void *dest, const void *src, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        *(char *) dest = ((char *) src)[i];
    }
    return dest;
}
#include <stdlib.h>

void *kmalloc(size_t size)
{
    (void) size;

    return NULL;
}

void kfree(void *ptr)
{
    (void) ptr;
}
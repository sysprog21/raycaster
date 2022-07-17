#ifndef MEM_H
#define MEM_H

#include <stddef.h>

void mem_init(void);

void *kmalloc(size_t size);

void kfree(void *ptr);

#endif  // MEM_H
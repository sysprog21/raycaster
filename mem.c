#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define HEAP_SIZE 1024 * 1024

typedef struct HeapSegmentHeader {
    uint32_t allocated;
    uint32_t size;
    struct HeapSegmentHeader *prev;
    struct HeapSegmentHeader *next;
} __attribute__((packed)) HeapSegmentHeader;

extern uint8_t __end[HEAP_SIZE];

static HeapSegmentHeader *heapSegmentsHead;

void mem_init(void)
{
    heapSegmentsHead = (HeapSegmentHeader *) &__end;
    heapSegmentsHead->allocated = false;
    heapSegmentsHead->size = HEAP_SIZE;
    heapSegmentsHead->prev = NULL;
    heapSegmentsHead->next = NULL;
}

void *malloc(size_t size)
{
    size += sizeof(HeapSegmentHeader);
    size += size % 16 ? 16 - (size % 16) : 0;

    HeapSegmentHeader *best = NULL;
    size_t bestDiff = SIZE_MAX;
    for (HeapSegmentHeader *curr = heapSegmentsHead; curr != NULL;
         curr = curr->next) {
        if (!curr->allocated && curr->size >= size) {
            size_t diff = curr->size - size;
            if (diff < bestDiff) {
                best = curr;
                bestDiff = diff;
            }
        }
    }

    if (best == NULL) {
        return best;
    }

    if (bestDiff > sizeof(HeapSegmentHeader) * 2) {
        HeapSegmentHeader *oldNext = best->next;
        best->next = (void *) best + size;
        best->next->allocated = false;
        best->next->size = best->size - size;
        best->next->next = oldNext;
        best->next->prev = best;
        best->size = size;
    }

    best->allocated = true;

    return best + 1;
}

void free(void *ptr)
{
    HeapSegmentHeader *segment = ptr - sizeof(HeapSegmentHeader);
    segment->allocated = false;
    while (segment->prev != NULL && !segment->prev->allocated) {
        segment->prev->next = segment->next;
        segment->prev->size += segment->size;
        segment = segment->prev;
    }
    while (segment->next != NULL && !segment->next->allocated) {
        segment->size += segment->next->size;
        segment->next = segment->next->next;
    }
}

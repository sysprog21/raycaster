#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define HEAP_SIZE 1024 * 1024

#define HeapSegmentHeader struct HeapSegmentHeader_

struct HeapSegmentHeader_ {
    bool allocated;
    size_t size;
    HeapSegmentHeader *prev;
    HeapSegmentHeader *next;
};

extern uint8_t __end;

static HeapSegmentHeader *heapSegmentsHead;

void mem_init(void)
{
    heapSegmentsHead = (HeapSegmentHeader *) &__end;
    heapSegmentsHead->allocated = false;
    heapSegmentsHead->size = HEAP_SIZE;
    heapSegmentsHead->prev = NULL;
    heapSegmentsHead->next = NULL;
}

void *kmalloc(size_t size)
{
    size += sizeof(HeapSegmentHeader);

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
        best->next->size = best->size - size;
        best->next->next = oldNext;
        best->next->prev = best;
        best->size = size;
    }

    best->allocated = true;

    return best;
}

void kfree(void *ptr)
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
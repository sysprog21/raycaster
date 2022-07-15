#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

typedef struct FbScreenSize {
    uint32_t width;
    uint32_t height;
} __attribute__((packed)) FbScreenSize;

typedef struct ClockRateSetInfo {
    uint32_t clockId;
    uint32_t rate;
    uint32_t skipTurbo;
} __attribute__((packed)) ClockRateSetInfo;

typedef struct FbAllocateRes {
    uint32_t base;
    uint32_t size;
} __attribute__((packed)) FbAllocateRes;

typedef struct ClockRateRes {
    uint32_t clockId;
    uint32_t rate;
} __attribute__((packed)) ClockRateRes;

typedef union ValueBuffer {
    FbScreenSize fbScreenSize;
    uint32_t fbBitsPerPixel;
    uint32_t fbAllocateAlignment;
    uint32_t clockId;
    ClockRateSetInfo clockRateSetInfo;

    FbAllocateRes fbAllocateRes;
    ClockRateRes clockRateRes;
} ValueBuffer;

#define NULL_TAG 0
#define FB_ALLOCATE_TAG 0x00040001
#define FB_SET_PHYSICAL_SIZE 0x00048003
#define FB_SET_VIRTUAL_SIZE 0x00048004
#define FB_SET_DEPTH 0x00048005
#define CLOCK_GET_MAX_RATE 0x00030004
#define CLOCK_SET_RATE 0x00038002

typedef struct PropertyMessageTag {
    uint32_t tag;
    ValueBuffer value;
} __attribute__((packed)) PropertyMessageTag;

uint32_t mailbox_read(uint8_t channel);

void mailbox_write(uint8_t channel, uint32_t data);

int mailbox_send_messages(PropertyMessageTag *tags);

#endif  // MAILBOX_H
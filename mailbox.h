#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

#define FbScreenSize struct FbScreenSize_

struct FbScreenSize_ {
    uint32_t width;
    uint32_t height;
} __attribute__((packed));

#define ClockRateSetInfo struct ClockRateSetInfo_

struct ClockRateSetInfo_ {
    uint32_t clockId;
    uint32_t rate;
    uint32_t skipTurbo;
} __attribute__((packed));

#define FbAllocateRes struct FbAllocateRes_

struct FbAllocateRes_ {
    uint32_t base;
    uint32_t size;
} __attribute__((packed));

#define ClockRateRes struct ClockRateRes_

struct ClockRateRes_ {
    uint32_t clockId;
    uint32_t rate;
} __attribute__((packed));

#define ValueBuffer union ValueBuffer_

union ValueBuffer_ {
    FbScreenSize fbScreenSize;
    uint32_t fbBitsPerPixel;
    uint32_t fbAllocateAlignment;
    uint32_t clockId;
    ClockRateSetInfo clockRateSetInfo;

    FbAllocateRes fbAllocateRes;
    ClockRateRes clockRateRes;
};

#define NULL_TAG 0
#define FB_ALLOCATE_TAG 0x00040001
#define FB_SET_PHYSICAL_SIZE 0x00048003
#define FB_SET_VIRTUAL_SIZE 0x00048004
#define FB_SET_DEPTH 0x00048005
#define CLOCK_GET_MAX_RATE 0x00030004
#define CLOCK_SET_RATE 0x00038002

#define PropertyMessageTag struct PropertyMessageTag_

struct PropertyMessageTag_ {
    uint32_t tag;
    ValueBuffer value;
} __attribute__((packed));

uint32_t mailbox_read(uint8_t channel);

void mailbox_write(uint8_t channel, uint32_t data);

int mailbox_send_messages(PropertyMessageTag *tags);

#endif  // MAILBOX_H
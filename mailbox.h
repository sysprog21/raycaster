#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

#define PropertyTag enum PropertyTag_

enum PropertyTag_ {
    NULL_TAG = 0,
    FB_ALLOCATE_TAG = 0x00040001,
    FB_SET_PHYSICAL_SIZE = 0x00048003,
    FB_SET_VIRTUAL_SIZE = 0x00048004,
    FB_SET_DEPTH = 0x00048005
};

#define FbScreenSize struct FbScreenSize_

struct FbScreenSize_ {
    uint32_t width;
    uint32_t height;
};

#define FbAllocateRes struct FbAllocateRes_

struct FbAllocateRes_ {
    uint32_t base;
    uint32_t size;
};

#define ValueBuffer union ValueBuffer_

union ValueBuffer_ {
    FbScreenSize fbScreenSize;
    uint32_t fbBitsPerPixel;
    uint32_t fbAllocateAlignment;

    FbAllocateRes fbAllocateRes;
};

#define PropertyMessage struct PropertyMessage_

struct PropertyMessage_ {
    PropertyTag tag;
    ValueBuffer value;
};

uint32_t mailbox_read(uint8_t channel);

void mailbox_write(uint8_t channel, uint32_t data);

void mailbox_send_messages(PropertyMessage *messages);

#endif  // MAILBOX_H
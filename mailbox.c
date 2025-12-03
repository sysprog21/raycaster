#include "mailbox.h"

#include <stdlib.h>
#include <string.h>

#include "mmio.h"

#define MAILBOX_BASE 0x2000B880
#define MAILBOX_READ (MAILBOX_BASE + 0x00)
#define MAILBOX_STATUS (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE (MAILBOX_BASE + 0x20)

#define MAIL_FULL 0x80000000
#define MAIL_EMPTY 0x40000000

#define MAILBOX_PROPERTY_CHANNEL 8

#define MESSAGE_CODE_REQUEST 0x00000000
#define MESSAGE_CODE_RESPONSE_SUCCESS 0x80000000
#define MESSAGE_CODE_RESPONSE_ERROR 0x80000001

typedef struct {
    uint32_t size;
    uint32_t code;
    uint32_t tags[1];
} __attribute__((packed)) PropertyMessageBuffer;

uint32_t mailbox_read(uint8_t channel)
{
    for (;;) {
        while (mmio_read(MAILBOX_STATUS) & MAIL_EMPTY)
            ;
        uint32_t data = mmio_read(MAILBOX_READ);
        uint8_t dataChannel = data & 0xF;
        data >>= 4;
        if (dataChannel == channel) {
            return data;
        }
    }
}

void mailbox_write(uint8_t channel, uint32_t data)
{
    while (mmio_read(MAILBOX_STATUS) & MAIL_FULL)
        ;
    mmio_write(MAILBOX_WRITE, data << 4 | channel);
}

uint32_t get_value_buffer_size(uint32_t tag)
{
    switch (tag) {
    case CLOCK_SET_RATE:
        return 12;
    case FB_ALLOCATE_TAG:
    case FB_SET_PHYSICAL_SIZE:
    case FB_SET_VIRTUAL_SIZE:
    case CLOCK_GET_MAX_RATE:
        return 8;
    case FB_SET_DEPTH:
        return 4;
    }
    return 0;
}

int mailbox_send_messages(PropertyMessageTag *tags)
{
    uint32_t size = 3 * sizeof(uint32_t), pos = 0;
    for (int i = 0; tags[i].tag != NULL_TAG; ++i) {
        size += get_value_buffer_size(tags[i].tag) + 3 * sizeof(uint32_t);
    }
    size += size % 16 ? 16 - (size % 16) : 0;

    PropertyMessageBuffer *buffer = malloc(size);
    if (!buffer)
        return -1;
    buffer->code = MESSAGE_CODE_REQUEST;
    buffer->size = size;
    for (int i = 0; tags[i].tag != NULL_TAG; ++i) {
        uint32_t length = get_value_buffer_size(tags[i].tag);
        buffer->tags[pos++] = tags[i].tag;
        buffer->tags[pos++] = length;
        buffer->tags[pos++] = 0;
        memcpy(&buffer->tags[pos], &tags[i].value, length);
        pos += length >> 2;
    }
    buffer->tags[pos] = NULL_TAG;

    mailbox_write(MAILBOX_PROPERTY_CHANNEL, (uint32_t) (uintptr_t) buffer >> 4);
    (void) mailbox_read(MAILBOX_PROPERTY_CHANNEL);

    if (buffer->code == MESSAGE_CODE_REQUEST) {
        free(buffer);
        return 1;
    }

    if (buffer->code == MESSAGE_CODE_RESPONSE_ERROR) {
        free(buffer);
        return 2;
    }

    pos = 0;
    for (int i = 0; tags[i].tag != NULL_TAG; ++i) {
        uint32_t length = get_value_buffer_size(tags[i].tag);
        pos += 3;
        memcpy(&tags[i].value, &buffer->tags[pos], length);
        pos += length >> 2;
    }

    free(buffer);
    return 0;
}

#include "mailbox.h"

#include "mmio.h"

#define MAILBOX_BASE 0x2000B880
#define MAILBOX_READ (MAILBOX_BASE + 0x00)
#define MAILBOX_STATUS (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE (MAILBOX_BASE + 0x20)

#define MAIL_FULL 0x80000000
#define MAIL_EMPTY 0x40000000

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
    mmio_write(MAILBOX_WRITE, (data << 4) | channel);
}

void mailbox_send_messages(PropertyMessage *messages) {}
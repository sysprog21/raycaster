#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdint.h>

uint32_t mailbox_read(uint8_t channel);

void mailbox_write(uint8_t channel, uint32_t data);

#endif  // MAILBOX_H
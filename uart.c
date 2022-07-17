#include "uart.h"

#include <stddef.h>

#include "mmio.h"

enum {
    GPIO_BASE = 0x20200000,

    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLK0 = (GPIO_BASE + 0x98),

    UART_BASE = 0x20201000,

    UART_DR = (UART_BASE + 0x00),
    UART_RSRECR = (UART_BASE + 0x04),
    UART_FR = (UART_BASE + 0x18),
    UART_ILPR = (UART_BASE + 0x20),
    UART_IBRD = (UART_BASE + 0x24),
    UART_FBRD = (UART_BASE + 0x28),
    UART_LCRH = (UART_BASE + 0x2C),
    UART_CR = (UART_BASE + 0x30),
    UART_IFLS = (UART_BASE + 0x34),
    UART_IMSC = (UART_BASE + 0x38),
    UART_RIS = (UART_BASE + 0x3C),
    UART_MIS = (UART_BASE + 0x40),
    UART_ICR = (UART_BASE + 0x44),
    UART_DMACR = (UART_BASE + 0x48),
    UART_ITCR = (UART_BASE + 0x80),
    UART_ITIP = (UART_BASE + 0x84),
    UART_ITOP = (UART_BASE + 0x88),
    UART_TDR = (UART_BASE + 0x8C),
};

void uart_init()
{
    mmio_write(UART_CR, 0x00000000);

    mmio_write(GPPUD, 0x00000000);
    delay(150);

    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    mmio_write(GPPUDCLK0, 0x00000000);

    mmio_write(UART_ICR, 0x7FF);

    mmio_write(UART_IBRD, 1);
    mmio_write(UART_FBRD, 40);

    mmio_write(UART_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    mmio_write(UART_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) |
                              (1 << 8) | (1 << 9) | (1 << 10));

    mmio_write(UART_CR, (1 << 0) | (1 << 8) | (1 << 9));
}

void uart_putc(char c)
{
    while (mmio_read(UART_FR) & (1 << 5))
        ;
    mmio_write(UART_DR, c);
}

char uart_getc()
{
    while (mmio_read(UART_FR) & (1 << 4))
        ;
    return mmio_read(UART_DR);
}

void uart_puts(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; ++i)
        uart_putc(str[i]);
}
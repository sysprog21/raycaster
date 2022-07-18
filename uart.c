#include "uart.h"

#include <stddef.h>

#include "mmio.h"

#define GPIO_BASE 0x20200000

#define GPIO_GPPUD (GPIO_BASE + 0x94)
#define GPIO_GPPUDCLK0 (GPIO_BASE + 0x98)

#define UART_BASE 0x20201000

#define UART_DR (UART_BASE + 0x00)
#define UART_RSRECR (UART_BASE + 0x04)
#define UART_FR (UART_BASE + 0x18)
#define UART_ILPR (UART_BASE + 0x20)
#define UART_IBRD (UART_BASE + 0x24)
#define UART_FBRD (UART_BASE + 0x28)
#define UART_LCRH (UART_BASE + 0x2C)
#define UART_CR (UART_BASE + 0x30)
#define UART_IFLS (UART_BASE + 0x34)
#define UART_IMSC (UART_BASE + 0x38)
#define UART_RIS (UART_BASE + 0x3C)
#define UART_MIS (UART_BASE + 0x40)
#define UART_ICR (UART_BASE + 0x44)
#define UART_DMACR (UART_BASE + 0x48)
#define UART_ITCR (UART_BASE + 0x80)
#define UART_ITIP (UART_BASE + 0x84)
#define UART_ITOP (UART_BASE + 0x88)
#define UART_TDR (UART_BASE + 0x8C)

#define UART_FULL 0x00000020
#define UART_EMPTY 0x00000010

void uart_init(void)
{
    mmio_write(UART_CR, 0x00000000);

    mmio_write(GPIO_GPPUD, 0x00000000);
    delay(150);

    mmio_write(GPIO_GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    mmio_write(GPIO_GPPUDCLK0, 0x00000000);

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
    while (mmio_read(UART_FR) & UART_FULL)
        ;
    mmio_write(UART_DR, c);
}

bool uart_empty(void)
{
    return mmio_read(UART_FR) & UART_EMPTY;
}

char uart_getc(void)
{
    while (mmio_read(UART_FR) & UART_EMPTY)
        ;
    return mmio_read(UART_DR);
}

void uart_puts(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; ++i)
        uart_putc(str[i]);
}
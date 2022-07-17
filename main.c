#include "uart.h"

void kmain()
{
    uart_init();
    uart_puts("Hello world!\n");
    while (1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }
}
#include "mem.h"
#include "uart.h"

void kmain()
{
    mem_init();
    uart_init();
    uart_puts("Hello world!\n");
    while (1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }
}
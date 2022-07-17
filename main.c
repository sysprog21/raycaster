#include <stdint.h>

#include "fb.h"
#include "mem.h"
#include "uart.h"

void kmain()
{
    mem_init();
    uart_init();
    uint32_t *fb = fb_create(640, 480, 32);
    if (fb == NULL) {
        uart_puts("Test\n");
    }
    uart_puts("Hello world!\n");
    while (1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }
}
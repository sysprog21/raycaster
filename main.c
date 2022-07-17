#include <stdint.h>

#include "fb.h"
#include "mem.h"
#include "uart.h"

void kmain()
{
    mem_init();
    uint32_t *fb = fb_create(1920, 1080, 4);
    if (fb) {
        uart_puts("Fuck\n");
    }
    uart_init();
    uart_puts("Hello world!\n");
    while (1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }
}
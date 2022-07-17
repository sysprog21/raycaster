#include <stdint.h>

#include "fb.h"
#include "mem.h"
#include "uart.h"

void kmain()
{
    mem_init();
    uart_init();
    uint32_t *fb = fb_create(640, 480, 16);
    for (int i = 0; i < 20000; ++i) {
        fb[i] = 0xFFFFFFFF;
    }
    uart_puts("Hello world!\n");
    while (1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }
}
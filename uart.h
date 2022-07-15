#ifndef UART_H
#define UART_H

#include <stdbool.h>

void uart_init(void);

bool uart_empty(void);

char uart_getc(void);

void uart_putc(char c);

void uart_puts(const char *str);

#endif  // UART_H
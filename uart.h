#ifndef UART_H
#define UART_H

void uart_init();

char uart_getc();

void uart_putc(char c);

void uart_puts(const char *str);

#endif  // UART_H
//https://www.nongnu.org/avr-libc/user-manual/group__stdiodemo.html


#ifndef _UART
#define _UART

#define RX_BUFSIZE 80
#define USE_BUSY_BIT 1

//out and in are usually stdout and stdin, but can be directed elsewhere if needed
void uart_init(unsigned int baud);

int uart_putchar(char c, FILE *stream);

int uart_getchar(FILE *stream);

#endif
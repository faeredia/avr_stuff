
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>

#include "uart.h"

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void uart_init(unsigned int baud){
    //http://wormfood.net/avrbaudcalc.php
    UCSR0A = _BV(U2X0);
    UBRR0L = (F_CPU / (8UL * baud)) - 1;
    UCSR0B = _BV(TXEN0) | _BV(RXEN0); /* tx/rx enable */

    stdout = stdin = &uart_str;
}

int uart_putchar(char c, FILE *stream){
  if (c == '\n')
    uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;

  return 0;
}

//placeholder for compatibility. not implented. 
int uart_getchar(FILE *stream){
    return 0;
}
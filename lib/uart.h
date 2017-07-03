#include <stdio.h>
#include <avr/io.h>

#ifndef BAUD
#define BAUD 19200
#endif

#define MYUBRR F_CPU/16/BAUD-1

void uart_init(unsigned int ubrr);

#include "uart.h"

// ** UART **

char uart_getchar(void)
{
	/* Wait for data to be received */
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

void uart_putchar(char c)
{
	 /* Wait until transmission ready. */
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

FILE uart_io = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void uart_init(unsigned int ubrr)
{
	//set speed
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)(ubrr);
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit - default*/
	stdin = &uart_io;
	stdout = &uart_io;
}

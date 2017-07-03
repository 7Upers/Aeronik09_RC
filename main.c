#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lib/uart.h"

#define LED PB5

#define IRLED PD6

#define IRL TCCR0B|=(1<<CS00)
#define IRH TCCR0B&=~(1<<CS00)

void ir_start(void)
{
	IRL;
	_delay_us(8500);
	IRH;
	_delay_us(4250);
}

void ir_stop(void)
{
	IRL;
	_delay_us(580);
	IRH;
}

void ir0(void)
{
	IRL;
	_delay_us(560);
	IRH;
	_delay_us(534);
}

void ir1(void)
{
	IRL;
	_delay_us(560);
	IRH;
	_delay_us(1600);
}

void ir_send(uint8_t *data)
{
	ir_start();
	uint8_t i = 7;
	while (i)
	{
		uint8_t currbyte = *data++;
		uint8_t j = 4;
		while (j)
		{
			if ((currbyte & 0x08) == 0x08)
			{
				ir1();
			}
			else
			{
				ir0();
			}
			currbyte = currbyte<<1;
			j--;
		}
		i--;
	}
	ir_stop();
}

uint8_t n = 0;
uint8_t acon[7] = {8,8,0,0,3,4,7};
uint8_t acoff[7] = {8,8,12,0,0,5,1};

int main (void)
{
	uart_init(MYUBRR);

	//sys led
	DDRB |= (1<<LED);
	PORTD &= ~(1<<LED);

	//ir out
	DDRD |= (1<<IRLED);
	PORTD |= (1<<IRLED);

	//setup timer
	//	generate carrier frequency
	TCCR0A |= ((1<<COM0A0)|(1<<WGM01));
	//TCCR0B |= (1<<CS00); //set devide 1  FREQ=16MHz
	OCR0A = 211; //FREQ=75829Hz  - toggle /2 = 37914Hz ~ 38KHz

	while (1)
	{
		_delay_ms(10000);
		ir_send((uint8_t *)&acon[0]);
		printf("%d ir transmitt ON ", n);
		_delay_ms(10000);
		ir_send((uint8_t *)&acoff[0]);
		printf("OFF\r\n");
		n++;
	}
}

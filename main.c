#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>


#include "lib/uart.h"
#include "lib/ds18b20.h"

#define LED PB5

#define IRLED PD6

#define IRL TCCR0B|=(1<<CS00)
#define IRH TCCR0B&=~(1<<CS00)

void ir_start(void)
{
	IRL;
	_delay_us(8970);
	IRH;
	_delay_us(4450);
}

void ir_stop(void)
{
	IRL;
	_delay_us(670);
	IRH;
}

void ir0(void)
{
	IRL;
	_delay_us(670);
	IRH;
	_delay_us(550);
}

void ir1(void)
{
	IRL;
	_delay_us(670);
	IRH;
	_delay_us(1622);
}

void ir_lp(void)
{
	//0
	IRL;
	_delay_us(670);
	IRH;
	_delay_us(550);
	//1
	IRL;
	_delay_us(670);
	IRH;
	_delay_us(1622);
	//LP
	IRL;
	_delay_us(670);
	IRH;
	_delay_us(19860);
}

void ir_send(uint8_t *data)
{
	ir_start();
	uint8_t i = 8;
	while (i)
	{
		//bytes cycle
		uint8_t currbyte = *data++;
		uint8_t j = 8;
		while (j)
		{
			//bits cycle
			if ((currbyte & 0x80) == 0x80)
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
		if ( i == 5 )
		{
			ir_lp();
		}
		i--;
	}
	ir_stop();
}

uint8_t n = 0;
uint8_t acon[8] = {0x90,0x10,0x06,0x0a,0x00,0x04,0x00,0x0b};
uint8_t acoff[8] = {0x80,0x10,0x06,0x0a,0x00,0x04,0x00,0xa};

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

	double temp = 0;
	uint8_t skip[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t sensor[8] = {0x28, 0x2d, 0xb2, 0x04, 0x05, 0x00, 0x00, 0x15};

	while (1)
	{
//		_delay_ms(10000);
//		ir_send((uint8_t *)&acon[0]);
//		printf("%d ir transmitt ON ", n);
//		_delay_ms(10000);
//		ir_send((uint8_t *)&acoff[0]);
//		printf("OFF\r\n");
		ds18b20_startconvert(skip);
		temp = ds18b20_gettemp(sensor);
		printf("Themperature= %3.2lf deg C\r\n", temp);
		_delay_ms(1000);
		n++;
	}
}

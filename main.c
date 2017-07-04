#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "lib/uart.h"
#include "lib/ds18b20.h"
#include "lib/aeronik09.h"

#define LED PB5
#define TOMAX 2400 //period of turn on air conditioner, 1 time peer 40min
#define TWMAX 1200 //max time of work air conditioner 20min
#define TCMAX 26 //max temperature of celsium degrees, after thet need turn on air conditioner
#define TCMIN 23 //min temperature of celsium degrees, after thet need turn off air conditioner

int main (void)
{
	uart_init(MYUBRR);

	//sys led
	DDRB |= (1<<LED);
	PORTD &= ~(1<<LED);

	aeronik09_init();

	uint8_t state = 0; //air condition 0 - turned off, 1 - turned on
	unsigned long int timer_o = TOMAX; //frequency of turn on control timer
	unsigned long int timer_w = 0; //work timer
	double temp = 0; //current temperature
	uint8_t skip[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	while (1)
	{
		ds18b20_startconvert(skip);
		temp = ds18b20_getsometemp();
		printf("Themperature= %3.2lf deg C (o=%ld, w=%ld)\r\n", temp, timer_o, timer_w);

		//Включаемся не чаще 1 раза в 40мин.
		if (( timer_o > TOMAX )&&( state == 0 ))
		{
			if ( temp > TCMAX )
			{
				aeronik09_on();
				_delay_ms(1000);
				aeronik09_on();
				printf("Aeronik09 ON\r\n");
				PORTB |= _BV(LED);
				timer_o = 0;
				state = 1;
			}
		}

		if ( state == 1 )
		{
			if (( timer_w > TWMAX )||( temp < TCMIN ))
			{
				aeronik09_off();
				_delay_ms(1000);
				aeronik09_off();
				printf("Aeronik09 OFF ");
				if ( temp < TCMIN )
				{
					printf(" by Temperature exceed\r\n");
				}
				else
				{
					printf(" by Timer exceed\r\n");
				}
				PORTB &= ~_BV(LED);
				timer_w = 0;
				state = 0;
			}
			timer_w++;
		}

		timer_o++;
		_delay_ms(500);
		//in real each cycle will be freq 1Hz (1time peer sec)
	}
}

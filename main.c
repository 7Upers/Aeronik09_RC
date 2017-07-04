#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>


#include "lib/uart.h"
#include "lib/ds18b20.h"
#include "lib/aeronik09.h"

#define LED PB5

int main (void)
{
	uart_init(MYUBRR);

	//sys led
	DDRB |= (1<<LED);
	PORTD &= ~(1<<LED);

	aeronik09_init();

	uint8_t state = 0;
	unsigned long int timer_o = 24; //frequency of turn on control timer
	unsigned long int timer_w = 0; //work timer
	double temp = 0;
	uint8_t skip[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	while (1)
	{
		ds18b20_startconvert(skip);
		temp = ds18b20_getsometemp();
		printf("Themperature= %3.2lf deg C (o=%ld, w=%ld)\r\n", temp, timer_o, timer_w);

		//Включаемся не чаще 1 раза в 40мин.
		if (( timer_o > 24 )&&( state == 0 ))
		{
			if ( temp > 25 )
			{
				aeronik09_on();
				printf("Aeronik09 ON\r\n");
				PORTB |= _BV(LED);
				state = 1;
			}
		}

		if ( state == 1 )
		{
			if (( timer_w > 12 )||( temp < 23 ))
			{
				aeronik09_off();
				printf("Aeronik09 OFF ");
				if ( temp < 23 )
				{
					printf(" by Temperature exceed\r\n");
				}
				else
				{
					printf(" by Timer exceed\r\n");
				}
				PORTB &= ~_BV(LED);
				timer_o = 0;
				timer_w = 0;
				state = 0;
			}
			timer_w++;
		}

		timer_o++;
		_delay_ms(1000);
	}
}

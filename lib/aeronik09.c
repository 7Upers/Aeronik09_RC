#include "aeronik09.h"

uint8_t acon[8] = {0x90,0x10,0x06,0x0a,0x00,0x04,0x00,0x0b};
uint8_t acoff[8] = {0x80,0x10,0x06,0x0a,0x00,0x04,0x00,0xa};

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
	_delay_us(L);
	IRH;
	_delay_us(H0);
}

void ir1(void)
{
	IRL;
	_delay_us(L);
	IRH;
	_delay_us(H1);
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
	//0
	IRL;
	_delay_us(670);
	IRH;
	_delay_us(550);
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
	uint8_t currbyte = 0;
	uint8_t j = 0;
	while (i)
	{
		//bytes cycle
		currbyte = *data++;
		j = 8;
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

void aeronik09_init(void)
{
	//ir out
	IRDDR |= (1<<IRLED);

	//setup timer
	//	generate carrier frequency
	TCCR0A |= (1<<WGM01); //CTC mode
	TCCR0B |= (1<<CS00); //set devide 1  FREQ=16MHz
#ifdef CFREQ38
	OCR0A = 211; //FREQ=75829Hz  - toggle /2 = 37914Hz ~ 38KHz
#else
#ifdef CFREQ36
	OCR0A = 222; //FREQ=72072Hz  - toggle /2 = 36036Hz ~ 36KHz
#else
	OCR0A = 160; //FREQ=100000Hz  - toggle /2 = 50000Hz = 50KHz
#endif
#endif
	IRH; //disable togle on compare - normal mode, turn off ir led
}

void aeronik09_on(void)
{
	ir_send((uint8_t *)&acon[0]);
}

void aeronik09_off(void)
{
	ir_send((uint8_t *)&acoff[0]);
}

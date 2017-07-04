/*
ds18b20 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "ds18b20.h"

#ifdef DEBUG1
#define DEBUG
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

/*
 * ds18b20 init
 */
uint8_t ds18b20_reset() {
	uint8_t i;

	//low for 480us
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(480);

	//release line and wait for 60uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(60);

	//get value and wait 420us
	i = (DS18B20_PIN & (1<<DS18B20_DQ));
	_delay_us(420);

	//return the read value, 0=ok, 1=error
	return i;
}

/*
 * write one bit
 */
void ds18b20_writebit(uint8_t bit){
	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		DS18B20_DDR &= ~(1<<DS18B20_DQ); //input

	//wait 60uS and release the line
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
}

/*
 * read one bit
 */
uint8_t ds18b20_readbit(void){
	uint8_t bit=0;

	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//release line and wait for 14uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(14);

	//read the value
	if(DS18B20_PIN & (1<<DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

/*
 * write one byte
 */
void ds18b20_writebyte(uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}

/*
 * read one byte
 */
uint8_t ds18b20_readbyte(void){
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit()<<7);
	}
	return n;
}

/*
 * check crc
 */
uint8_t crc8( uint8_t *addr, uint8_t len)
{
	uint8_t crc=0;
	uint8_t i=0;

	while ( i<len)
	{
		uint8_t inbyte = addr[i];
		uint8_t j=0;
		while ( j<8 )
		{
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
			j++;
		}
		i++;
	}
	return crc;
}

uint8_t ds18b20_startconvert(uint8_t *addr)
{
	ds18b20_reset(); //reset

	if ( addr[0] == 0x00 )
	{
		ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	}
	else
	{
		#ifdef DEBUG
		printf("CONVERTTEMP ROM = ");
		#endif
		uint8_t i = 0;
		while ( i < 8 )
		{
			ds18b20_writebyte(addr[i]);
			i++;
			#ifdef DEBUG
			printf("%02xh ",addr[i]);
			#endif
		}
		#ifdef DEBUG
		printf("\r\n");
		#endif
	}

	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP); //start temperature conversion

/*
	Thermometer	Max Conversion
	Resolution	Time
	9 bit		93.75 ms (t conv/8)
	10 bit		187.5 ms (t conv/4)
	11 bit		375 ms (t conv/2)
	12 bit		750 ms (t conv)
*/

//don't need wait until conversation in this project
/*
	uint8_t ttl = 16; // 16*50ms = 800ms > max time conversion
	while(!ds18b20_readbit()) //wait until conversion is complete or ttl will be empty
	{
		if ( ! ttl )
		{
			return 1; // fail
		}
		ttl--;
		_delay_ms(50);
	}
*/
	#ifdef DEBUG
	printf("conversion complete less than %dms\r\n",((16-ttl)*50));
	#endif

	return 0; //success
}

double ds18b20_gettemp(uint8_t *addr)
{
	uint8_t temperature_l;
	uint8_t temperature_h;
	double retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_MATCHROM); //match ROM

	#ifdef DEBUG
	printf("match ROM = ");
	#endif
	uint8_t i = 0;
	while ( i < 8 )
	{
		ds18b20_writebyte(addr[i]);
		i++;
		#ifdef DEBUG
		printf("%02xh ",addr[i]);
		#endif
	}
	#ifdef DEBUG
	printf("\r\n");
	#endif

	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 2 byte from scratchpad
	temperature_l = ds18b20_readbyte();
	temperature_h = ds18b20_readbyte();

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	uint8_t order = ( temperature_h << 4 ) + ( temperature_l >> 4 );
	if ( order & 0b10000000 ) order = ~order; //старший бит == 1 значит температура отрицательная

	#ifdef DEBUG
	printf("order=%d\r\n",order);
	#endif

	double mantissa = ( temperature_l & 0b00001111 ) * 0.0625;

	#ifdef DEBUG
	printf("mantissa=%fl\r\n",mantissa);
	#endif


	if ( temperature_h & 0b10000000 )
	{
		retd = -1 * ( order + ( 1 - mantissa ) );
	}
	else
	{
		retd = order + mantissa ;
	}

	#ifdef DEBUG
	printf("rezult=%lf\r\n", retd);
	#endif


	return retd;
}

/*
 * get temperature
 */
double ds18b20_getsometemp() {
	uint8_t temperature_l;
	uint8_t temperature_h;
	double retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP); //start temperature conversion

	while(!ds18b20_readbit()); //wait until conversion is complete

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 2 byte from scratchpad
	temperature_l = ds18b20_readbyte();
	temperature_h = ds18b20_readbyte();

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	//convert the 12 bit value obtained
//	retd = ( ( temperature_h << 8 ) + temperature_l ) * 0.0625;

	if ( temperature_h & 0b10000000 )
	{
		temperature_h = ~temperature_h;
		temperature_l = ~temperature_l;
//		retd = -1 * ((( temperature_h << 8 ) + temperature_l + 1) >> 4 ); // devide by 16 to integer
		retd = -1 * (( temperature_h << 8 ) + temperature_l + 1) * 0.0625;
	}
	else
	{
//		retd = (( temperature_h << 8 ) + temperature_l ) >> 4;
		retd = (( temperature_h << 8 ) + temperature_l ) * 0.0625;
	}


	return retd;
}

void ds18b20_getaddr(uint8_t *sn)
{
	uint8_t i = 0;

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_READROM); //read rom

	//read 8 bytes from rom (Family code 0x28 [1 byte] + serial number [6 bytes] + CRC [1 byte])
	while ( i < 8 )
	{
		sn[i] = ds18b20_readbyte();
		i++;
	}
}

void ds18b20_search(uint8_t sensors[][8])
{
	uint8_t sensor_number = 0; // счетчик найденных датчиков
	uint8_t conf_bit = 0; // позиция бита в серийном номере на котором конфликт битов адресов датчиков
	uint8_t p_conf_bit = 0; // позиция бита в ПОСЛЕДНЕМ серийном номере на котором конфликт битов адресов датчиков, если конфликтов небыло т.е. ==0 - то больше не нужно продолжать искать датчики

	do
	{
		ds18b20_reset(); //reset
		ds18b20_writebyte(DS18B20_CMD_SEARCHROM); //search rom

//		uint8_t k = 0; // счетчик байт сохраняемой части адреса (без первого байта 0x28 Family code)
		uint8_t i = 0; // счетчик байт передаваемого адреса
		uint8_t curr_bit = 0; // счетчик бит в серийном номере - сквозная нумерация

#ifdef DEBUG
#ifndef DEBUG1
		printf("sensor[%d] s/n: ",sensor_number);
#endif
#endif

		while ( i < 8 )
		{
			uint8_t buf = 0; //буффер для получения очередного байта адреса
			uint8_t z = 0; //бит адреса переданный от датчиков
			uint8_t nz = 0; //бит адреса переданный от датчикв в инверсном виде
			uint8_t j = 0; //счетчик бит - очередного байта адреса
			
			while ( j < 8 )
			{
				z = ds18b20_readbit(); //читаем бит
				nz = ds18b20_readbit(); //читаем инвертированный бит
#ifdef DEBUG1
				printf ("%02d:z=%1d,nz=%1d",curr_bit,z,nz);
#endif
				if ( z != nz )
				{
					//на шине нет датчиков с разными битами в этой позиции
					ds18b20_writebit(z); // записываем бит на шину в не инвертированном виде для подтверждения
					buf = (buf >> 1) + (z << 7); // двигаем буффер вправо (т.к. передаются от младшего к старшему) и добавляем очередной переданный бит в старшую позицию
#ifdef DEBUG1
					printf(".%1d ",z); //noconflict
#endif
				}
				if (( z == 1 )&&( nz == 1 ))
				{
					//датчики с таким переданным битом в этой позиции к шине не подключенны
					//выход к новому поиску
					return; // временно
				}
				if (( z == 0 )&&( nz == 0 ))
				{
					//на шине есть датчики с разными битами в этой позиции
					if ( curr_bit == p_conf_bit )
					{
						ds18b20_writebit(1);
						buf = (buf >> 1) + 0x80; // двигаем буффер вправо (т.к. передаются от младшего к старшему) и добавляем очередной переданный бит в старшую позицию
#ifdef DEBUG1
						printf("^1 "); //unconflict
#endif
					}
					else if ( curr_bit < p_conf_bit )
					{
						if ( ((sensors[sensor_number-1][i] >> (curr_bit - i*8))&0x01) == 1 )
						{
							ds18b20_writebit(1);
							buf = (buf >> 1) + 0x80; // двигаем буффер вправо (т.к. передаются от младшего к старшему) и добавляем очередной переданный бит в старшую позицию
#ifdef DEBUG1
							printf(">1 "); //unconflict
#endif
						}
						else
						{
							ds18b20_writebit(0);
							buf = (buf >> 1); // двигаем буффер вправо (т.к. передаются от младшего к старшему) и добавляем очередной переданный бит в старшую позицию
							conf_bit = curr_bit;
#ifdef DEBUG1
							printf("?0 "); //conflict
#endif
						}
					}
					else
					{
						ds18b20_writebit(0);
						buf = (buf >> 1); // двигаем буффер вправо (т.к. передаются от младшего к старшему) и добавляем очередной переданный бит в старшую позицию
						conf_bit = curr_bit;
#ifdef DEBUG1
						printf("!0 "); //conflict
#endif
					}
				}
				//инкремент номера бита
				j++;
				curr_bit++;
			}
#ifdef DEBUG1
			printf(" buf=");
#endif
#ifdef DEBUG
			printf("%02xh ",buf);
#endif
#ifdef DEBUG1
			printf("\r\n");
#endif
			// сохраняем байт адреса в массиве для возврата
//			if (( i >= 1 )&&( i <= 6 ))
//			{
//				sensors[sensor_number][k] = buf;
				sensors[sensor_number][i] = buf;
//				k++;
//			}
			i++;
		}

		p_conf_bit = conf_bit; //сохраняем позицию последнего конфликтного бита
		conf_bit = 0; //очистка бувера конфликтного бита
		sensor_number++;

#ifdef DEBUG1
		printf("conf_bit=%d",p_conf_bit);
#endif
#ifdef DEBUG
		printf("\r\n");
#endif
	}
	while ( p_conf_bit != 0 );
}

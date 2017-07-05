#include <avr/io.h>
#include <util/delay.h>

#define IRDDR DDRD
#define IRPORT PORTD
#define IRLED PD6

//carrier frequency (CFREQ36 = 36kHz, CFREQ38 = 38kHz, any else or CFREQ50 = 50kHz)
#define CFREQ50 1

//#define IRL TCCR0B|=(1<<CS00)
#define IRL TCCR0A|=_BV(COM0A0); //enable toggle on compare
//#define IRH TCCR0B&=~(1<<CS00); IRPORT &= ~_BV(IRLED)
#define IRH TCCR0A&=~_BV(COM0A0); IRPORT&=~_BV(IRLED); //disable toggle on compare - normal mode, turn off ir led

#define L 670 - 50
#define H0 550 + 30
#define H1 1620 + 70

void ir_start(void);
void ir_stop(void);
void ir0(void);
void ir1(void);
void ir_lp(void);
void ir_send(uint8_t *data);
void aeronik09_init(void);
void aeronik09_on(void);
void aeronik09_off(void);

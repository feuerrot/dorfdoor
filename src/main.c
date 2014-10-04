// coding: utf-8
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "rs232.h"

char resettext[]	=	"p";	// power on
char openhs[]		=	"o";	// hackerspace open
char closedhs[]		=	"c";	// hackerspace closed
char dooror1[]		=	"1";	// ssh stage 1
char dooror2[]		=	"2";	// ssh stage 2
char dooror3[]		=	"e";	// ssh error
char dooropened[]	=	"s";	// open via switch
char bell[]		=	"b";	// bell
char resetof[]		=	"r";	// ssh stage reset
char statuso[]		=	"SO";
char statusc[]		=	"SC";

volatile uint8_t resetoverflow;
uint8_t toggledoor;
#define DOORTOGGLE 20

#define SWITCH_MODE	PD5
#define SWITCH_OUT	PD6
#define SWITCH_IN	PD7
#define DOOR_DATA	PC2
#define DOOR_POWER	PC0
#define DOOR_OPEN	PC1
#define LED_OUT		PC4
#define LED_IN		PC3

volatile struct {
	unsigned dooropenstage1:1;
	unsigned dooropenstage2:1;
	unsigned openhackerspace:1;
	unsigned openhackerspaceold:1;
	unsigned bell:1;
} flags;

void init(void){
	_usart_init();

	/* Protipp:
	 * Der Defaultzustand von PORT- und DDR-Registern ist 0x00.
	 * D.h.: Schaltfunktion bei Output = low ist eine scheiß Idee.
	 *
	 * Hier fixen wir das, indem wir erstmal einen Pullup machen und
	 * dann den Pin auf high schalten.
	 */
	PORTC |= (1<<DOOR_OPEN);
	DDRC  |= (1<<DOOR_OPEN);

	DDRC  |= (1<<DOOR_DATA)|(1<<DOOR_POWER)|(1<<LED_OUT)|(1<<LED_IN);
	PORTC |= (1<<DOOR_DATA)|(1<<LED_OUT)|(1<<LED_IN);

	PORTD |= (1<<SWITCH_MODE)|(1<<SWITCH_OUT)|(1<<SWITCH_IN);

	_delay_ms(10);
	TCCR2B |= (1<<CS22)|(1<<CS21);
	TIMSK2 |= (1<<TOIE2);

	sei();
	_serputs(resettext);
}

void setled(uint8_t led){
	if (led){
		PORTC &= ~((1<<LED_OUT)|(1<<LED_IN));
	} else {
		PORTC |= (1<<LED_OUT)|(1<<LED_IN);
	}
}

void opendoor(void){
	PORTC &= ~(1<<DOOR_OPEN);
	_delay_ms(100);
	PORTC |= (1<<DOOR_OPEN);
	_serputs(dooropened);
}

uint8_t input(uint8_t flag){
	return (!(PIND & (1<<flag)));
}

int main(void) {
	init();

	while(1) {
		if (flags.openhackerspace != flags.openhackerspaceold){
			flags.openhackerspaceold = flags.openhackerspace;
			if (flags.openhackerspace){
				setled(1);
				_serputs(openhs);
			} else {
				setled(0);
				_serputs(closedhs);
			}
		}
		if (flags.openhackerspace){
			if (input(SWITCH_IN) | input(SWITCH_OUT)){
				setled(0);
				_delay_ms(50);
				setled(1);
				opendoor();
				setled(0);
				_delay_ms(50);
				setled(1);
			}
		} else {
			if ((input(SWITCH_IN) | input(SWITCH_OUT)) && flags.bell == 0){
				_serputs(bell);
				flags.bell = 1;
				//Klingelfunktion
			}
		}
		if (flags.dooropenstage2){
			_serputs(dooror2);
			flags.dooropenstage2 = 0;
			opendoor();
		}

		if (input(SWITCH_MODE)){
			toggledoor++;
		} else {
			toggledoor = 0;
		}
		if (toggledoor > DOORTOGGLE){
			flags.openhackerspace ^= 1;
			toggledoor = 0;
		}

		_delay_ms(100);
	}
	return 0;
}

ISR(USART_RX_vect){
    uint8_t chr = UDR0;
    switch (chr){
		case 'r':
		       	wdt_enable(WDTO_1S);
			break;
		case 'd':
			if (flags.dooropenstage1){
				_serputs(dooror3);
			} else {
				flags.dooropenstage1 = 1;
				_serputs(dooror1);
			}
			break;
		case 'b':
			if (flags.dooropenstage1){
				flags.dooropenstage2 = 1;
				flags.dooropenstage1 = 0;
			} else {
				_serputs(dooror3);
			}
			break;
		case 'o':
			flags.openhackerspace = 1;
			break;
		case 'c':
			flags.openhackerspace = 0;
			break;
		case 's':
			if (flags.openhackerspace){
				_serputs(statuso);
			} else {
				_serputs(statusc);
			}
		default:
			flags.dooropenstage1 = 0;
			break;
    }
}

ISR(TIMER2_OVF_vect){
	if (resetoverflow == 0){
		flags.dooropenstage1 = 0;
		flags.dooropenstage2 = 0;
		flags.bell = 0;
		_serputs(resetof);
	}
	resetoverflow++;
}

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

#define SWITCH_RED	PD2 // Taster Rot
#define SWITCH_GREEN	PD5 // Taster Grün
#define SWITCH_OUT	PD6
#define SWITCH_IN	PD7
#define DOOR_DATA	PC2
#define DOOR_POWER	PC0
#define DOOR_OPEN	PC1
#define LED_OUT		PC4 // normale LED im Außentaster
#define LED_IN		PC3 // sind die WS2812b

volatile struct {
	unsigned dooropenstage1:1;
	unsigned dooropenstage2:1;
	unsigned openhackerspace:1;
	unsigned openhackerspaceold:1;
	unsigned bell:1;
} flags;

#define CMD_OPEN_LOCK      0x10cd
#define CMD_OPEN_TF        0x11d0
#define CMD_OPEN_TF_HOLD   0x12f7
#define CMD_LOCK           0x14b9
#define CMD_UNLOCK         0x179e

void ws2812bit(uint8_t bit){
	PORTC |= (1<<LED_IN);
	if (bit){
		_delay_us(0.85);
		PORTC &= ~(1<<LED_IN);
		_delay_us(0.35);
	} else {
		_delay_us(0.35);
		PORTC &= ~(1<<LED_IN);
		_delay_us(0.85);
	}
}

void led_r(void){
	for (int i=7; i>=0; i--){
		ws2812bit(0);
	}
	for (int i=7; i>=0; i--){
		ws2812bit(1);
	}
	for (int i=7; i>=0; i--){
		ws2812bit(0);
	}
}

void led_g(void){
	for (int i=7; i>=0; i--){
		ws2812bit(1);
	}
	for (int i=13; i>=0; i--){
		ws2812bit(0);
	}
}


static void send_door_halfbit(int v) {
	if (!v) {
		PORTC &= ~(1 << DOOR_DATA);
	} else {
		PORTC |= (1 << DOOR_DATA);
	}
	_delay_us(500);
}

static void send_door_cmd(uint16_t word) {
	cli();

	for (int rep = 1; rep <= 11; rep++) {
		send_door_halfbit(0);

		for (signed int i = 15; i >= 0; i--) {
			if (word & ((uint16_t)1 << i)) {
				/* bit set */
				send_door_halfbit(1);
				send_door_halfbit(0);
			} else {
				send_door_halfbit(0);
				send_door_halfbit(1);
			}
		}

		/* release bus */
		send_door_halfbit(1);

		/* packet delay */
		_delay_us(2500);
	}

	sei();
}

void setled(uint8_t led){
	cli();
	if (led){
		PORTC &= ~(1<<LED_OUT);
		led_g();
		led_g();
		led_g();
	} else {
		PORTC |= (1<<LED_OUT);
		led_r();
		led_r();
		led_r();
	}
	sei();
}

void opendoor(void){
	if (flags.openhackerspace) {
		send_door_cmd(CMD_OPEN_TF);
	} else {
		send_door_cmd(CMD_OPEN_LOCK);
	}
	_delay_ms(100);
	_serputs(dooropened);
}

uint8_t input(uint8_t flag){
	return (!(PIND & (1<<flag)));
}

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

	PORTD |= (1<<SWITCH_GREEN)|(1<<SWITCH_RED)|(1<<SWITCH_OUT)|(1<<SWITCH_IN);

	_delay_ms(10);
	TCCR2B |= (1<<CS22)|(1<<CS21);
	TIMSK2 |= (1<<TOIE2);

	setled(0);

	sei();
	_serputs(resettext);
}

int main(void) {
	init();

	while(1) {
		if (flags.openhackerspace != flags.openhackerspaceold){
			flags.openhackerspaceold = flags.openhackerspace;
			if (flags.openhackerspace){
				send_door_cmd(CMD_UNLOCK);
				setled(1);
				_serputs(openhs);
			} else {
				send_door_cmd(CMD_LOCK);
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

		if (input(SWITCH_GREEN)|input(SWITCH_RED)){
			toggledoor++;
		} else {
			toggledoor = 0;
		}
		if (toggledoor > DOORTOGGLE){
			if ((flags.openhackerspace == 0 && input(SWITCH_GREEN))||(flags.openhackerspace == 1 && input(SWITCH_RED))){
				flags.openhackerspace ^= 1;
			}
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

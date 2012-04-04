// coding: utf-8
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define DEBUG 0

#define ENC_A (PIND & (1<<PIND5))
#define ENC_B (PIND & (1<<PIND6))
#define ENC_T (PIND & (1<<PIND7))
volatile uint8_t enc_delta;
volatile uint8_t lastchar;
volatile uint8_t resetoverflow;

#if DEBUG
	char resettext[]	=	"[x] Reset\n";
	char openhs[]		=	"* hackerspace opened\n";
	char closedhs[]		=	"* hackerspace closed\n";
	char dooror1[]		=	"* ssh stage 1\n";
	char dooror2[]		=	"* ssh stage 2, opened\n";
	char dooror3[]		=	"* stage error\n";
	char dooropened[]	=	"* door opened via switch\n";
	char bell[]			=	"* someone is at the door\n";
	char resetof[] 		=	"* stage resetted\n";
	char statuso[]		=	"STATUS: OPEN\n";
	char status[]		=	"STATUS: CLOSED\n";
#else
	char resettext[]	=	"p";	// power on
	char openhs[]		=	"o";	// hackerspace open
	char closedhs[]		=	"c";	// hackerspace closed
	char dooror1[]		=	"1";	// ssh stage 1
	char dooror2[]		=	"2";	// ssh stage 2
	char dooror3[]		=	"e";	// ssh error
	char dooropened[]	=	"s";	// open via switch
	char bell[]			=	"b";	// bell
	char resetof[]		=	"r";	// ssh stage reset
	char statuso[]		=	"SO";
	char statusc[]		=	"SC";
#endif

uint8_t toggledoor;
#define DOORTOGGLE 20

volatile struct {
	unsigned dooropenstage1:1;
	unsigned dooropenstage2:1;
	unsigned openhackerspace:1;
	unsigned openhackerspaceold:1;
	unsigned bell:1;
} flags;

uint8_t enc_delta_old;

void init(void){
	usart_init();

	DDRC  |= (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3);
	DDRD  |= (1<<PD3)|(1<<PD4);
	DDRD  &= ~((1<<PD2)|(1<<PD5)|(1<<PD6)|(1<<PD7));
	PORTD |= (1<<PD2)|(1<<PD5)|(1<<PD6)|(1<<PD7);

	_delay_ms(10);
	TCCR0 |= (1<<CS01);
	TIMSK |= (1<<TOIE0);

	TCCR2 |= (1<<CS22)|(1<<CS21);
	TIMSK |= (1<<TOIE2);

	sei();
	serputs(resettext);
}

void laa(uint8_t state){
	if (state)
		PORTC |= 0b1111;
	else
		PORTC &= ~0b1111;
}

int main(void) {
	init();

	while(1) {
		if (flags.openhackerspace != flags.openhackerspaceold){
			flags.openhackerspaceold = flags.openhackerspace;
			if (flags.openhackerspace){
				PORTD |= (1<<PD4);
				laa(1);
				serputs(openhs);
			} else {
				PORTD &= ~(1<<PD4);
				laa(0);
				serputs(closedhs);
			}
		}
		if (flags.openhackerspace){
			if (!(PIND & (1<<PD2))){
				PORTD |= (1<<PD3);
				_delay_ms(10);
				PORTD &= ~(1<<PD4);
				laa(0);
				_delay_ms(90);
				PORTD &= ~(1<<PD3);
				_delay_ms(300);
				PORTD |= (1<<PD4);
				laa(1);
				serputs(dooropened);
			}
		} else {
			if (!(PIND & (1<<PD2)) && flags.bell == 0){
				serputs(bell);
				flags.bell = 1;
				//Klingelfunktion
			}
		}
		if (flags.dooropenstage2){
			serputs(dooror2);
			flags.dooropenstage2 = 0;
			PORTD |= (1<<PD3);
			_delay_ms(100);
			PORTD &= ~(1<<PD3);
		}

		if (ENC_T){
			toggledoor = 0;
		} else {
			toggledoor++;
		}
		if (toggledoor > DOORTOGGLE){
			flags.openhackerspace ^= 1;
			toggledoor = 0;
		}

		_delay_ms(100);
	}
	return 0;
}

ISR(USART_RXC_vect){
    uint8_t chr = UDR;
    switch (chr){
		case 'r':
        	wdt_enable(WDTO_1S);
			break;
		case 'd':
			if (flags.dooropenstage1){
				serputs(dooror3);
			} else {
				flags.dooropenstage1 = 1;
				serputs(dooror1);
			}
			break;
		case 'b':
			if (flags.dooropenstage1){
				flags.dooropenstage2 = 1;
				flags.dooropenstage1 = 0;
			} else {
				serputs(dooror3);
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
				serputs(statuso);
			} else {
				serputs(statusc);
			}
		default:
			flags.dooropenstage1 = 0;
			break;
    }
}

ISR(TIMER0_OVF_vect){
	static char enc_last = 0x01;
	char i = 0;
	if(ENC_A)
		i = 1;
	if(ENC_B)
		i ^= 3;						// convert gray to binary
	i -= enc_last;					// difference new - last
	if( i & 1 ){					// bit 0 = value (1)
		enc_last += i;				// store new as next last
		enc_delta += (i & 2) - 1;	// bit 1 = direction (+/-)
	}
}

ISR(TIMER2_OVF_vect){
	if (resetoverflow == 0){
		flags.dooropenstage1 = 0;
		flags.dooropenstage2 = 0;
		flags.bell = 0;
		serputs(resetof);
	}
	resetoverflow++;
}

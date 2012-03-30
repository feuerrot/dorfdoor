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

char resettext[] = "[x] Reset\n";
char openhs[] = "* hackerspace opened\n";
char closedhs[] = "* hackerspace closed\n";
char dooror1[] = "* door open via ssh stage 1\n";
char dooror2[] = "* door open via ssh stage 2, opened\n";
char dooror3[] = "* door open via ssh error, stage 2 before stage 1 or two times stage 1!\n";
char dooropened[] = "* door opened via switch\n";
char bell[] = "* someone is at the door\n";
char resetof[] = "* stage resetted\n";

volatile struct {
	unsigned dooropenstage1:1;
	unsigned dooropenstage2:1;
	unsigned openhackerspace:1;
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

int main(void) {
	init();

	while(1) {
		if (flags.openhackerspace){
			if (!(PIND & (1<<PD2))){
				PORTD |= (1<<PD3);
				_delay_ms(10);
				PORTD &= ~(1<<PD4);
				_delay_ms(90);
				PORTD &= ~(1<<PD3);
				_delay_ms(300);
				PORTD |= (1<<PD4);
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
				#if DEBUG
				serputs(dooror3);
				#endif
			} else {
				flags.dooropenstage1 = 1;
				#if DEBUG
				serputs(dooror1);
				#endif
			}
			break;
		case 'b':
			if (flags.dooropenstage1){
				flags.dooropenstage2 = 1;
				flags.dooropenstage1 = 0;
			} else {
				#if DEBUG
				serputs(dooror3);
				#endif
			}
			break;
		case 'o':
			flags.dooropenstage1 = 0;
			flags.openhackerspace = 1;
			PORTD |= (1<<PD4);
			serputs(openhs);
			break;
		case 'c':
			flags.dooropenstage1 = 0;
			flags.openhackerspace = 0;
			PORTD &= ~(1<<PD4);
			serputs(closedhs);
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
		#if DEBUG
		serputs(resetof);
		#endif
	}
	resetoverflow++;
}

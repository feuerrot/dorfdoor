// coding: utf-8
#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD 115200UL      // Baudrate
#include <util/setbaud.h>  // Das util setzt die Baudrate automatisch

void serputs(char *s){
    while (*s){
        while (!(UCSRA & (1<<UDRE))){}
        UDR = *s;
        s++;
    }
}

void usart_init(void){ //USART init
    UBRRH = UBRRH_VALUE;  // Übernimmt die Werte von util/setbaud.h in das passende Register
    UBRRL = UBRRL_VALUE;  // Übernimmt die Werte von util/setbaud.h in das passende Register
    #if USE_2X  // U2X-Modus erforderlich
        UCSRA |= (1 << U2X);
    #else       // U2X-Modus nicht erforderlich
        UCSRA &= ~(1 << U2X);
    #endif
    UCSRB |= (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);  //RX und TX enabled, Interrupt bei RX enabled
    UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}

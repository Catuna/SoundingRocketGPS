/**
 * \file
 *
 * \brief Empty user application template
 *
 */

#define F_CPU 16000000UL // IMPORTANT: Replace this with frequency of occilator in final version


/* The largest size NMEA sentence that should be storable */
#define NMEA_BUFFER_SIZE 82

/* The GPGGA sentence to send if no actual GPS data has been recieved yet */
#define INITIAL_GPGGA "$GPGGA,777777.777,7777.7777,N,77777.7777,W,7,7,7.77,77.7,M,77.7,M,,*77"
#define INITIAL_GPGGA_SIZE 70

#define OUTPUT_PORT PORTA //TODO: Not sure which output reg to use
#define LENGTH_OF_PARSED 26 // Length of parsed char array
#define LENGTH_OF_OUTPUT_MESSAGE 14 // Number of uint8_t in one whole message including message separators (2x uint8_t)


#include <asf.h>
#include <inttypes.h>
#include <string.h>
#include <avr/sleep.h>

#include "parser.h"

int is_reading_sentence = 0; // 1 if buffer is currently reading a valid NMEA sentence

// Store buffer has the newest complete GPGGA message
char buffer1[82] = INITIAL_GPGGA;
char* stored_ptr = buffer1;
uint8_t stored_size = INITIAL_GPGGA_SIZE;

// Receive buffer stores the incoming NMEA message
char buffer2[NMEA_BUFFER_SIZE];
char* receiving_ptr = buffer2;
uint8_t receiving_size;

static void USART_Init(void) {
	/* Hold reset on GPS until USART has finished initialization */
	PORTB &= ~(1<<DDB0);

	/* Calculate baud-register values */
	#define BAUD 9600
	#define USE_2X 0
	#include <util/setbaud.h>

	/* Set baud registers for USART */
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

	/* Enable receiver and transmitter */
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8 data, 1 stop bit */
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01);
	
	/* Enable the UART0 receive complete interrupt */
	UCSR0B |= (1 << RXCIE0);

	/* Release reset on GPS */
	PORTB |= 0x01;
}

static void GPIO_Init(void) {
	//TODO: GPIO interrupt enable and pin configuration here
	
	/* Set PB0 to output (Use for reset signal on GPS) */
	DDRB |= (1<<DDB0); 
	
	/* Set PB1 to input, (FrameStart pin) */
	DDRB &= ~(1<<DDB1);
	
	/* Set PA0-7 to output (Used for sending 1 byte)*/
	DDRA = 0xff;
	
	/* Enable interrupt on pin PB1 (apparently not) */
	PCMSK1 |=  1 << PCINT9;
	
	/* Enable interrupt on port B */
	PCICR |= 1 << PCIE1;
	
	/* Set interrupt to trigger on rising edge */
	EICRA |= ((1 << ISC10)	| (1 << ISC11)); 
	
}


ISR(USART0_RX_vect) {
	
    /* Store received character */
    char msg = UDR0;

    /* Check for start of sentence */
    if (msg == '$') {
        // Only replace content of the stored buffer if the new data is a GPS fix sentence
        if (strncmp(receiving_ptr, "$GPGGA", 6) == 0) {
            char* old_stored = stored_ptr;
            stored_ptr = receiving_ptr;
            receiving_ptr = old_stored;
			stored_size = receiving_size;
        }
        receiving_size = 0;
    }
    if (receiving_size != NMEA_BUFFER_SIZE) {
        /* Put new character at the correct offset from the receive buffer */
        *(receiving_ptr+receiving_size) = msg;
        receiving_size++;
    }
}

uint8_t next_byte_to_send = LENGTH_OF_OUTPUT_MESSAGE; // Initialized to 15 such that we will load a new message the first time.
uint8_t array_currently_being_sent[14];

//PB1 - PCINT9
ISR(PCINT1_vect) {
	// Check if it is a rising edge. (we don't care about falling edge)
	if (PINB & (1 << PB1)){
		// Check if we are done sending the whole message. 
		//		Update array_currently_being_sent with the newest available message from GPS.
	if (next_byte_to_send >= LENGTH_OF_OUTPUT_MESSAGE) {
		
		char parsed[LENGTH_OF_PARSED] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
		parse_NMEA(stored_ptr, stored_size, parsed);
		truncate_char_array(parsed, LENGTH_OF_PARSED, array_currently_being_sent);
		next_byte_to_send = 0;
	}
		
	//Send the next byte/uint8_t.
	OUTPUT_PORT = get_output_data(array_currently_being_sent[next_byte_to_send]);
	next_byte_to_send++;
    }    
}

int main (void)
{
	cli();
	GPIO_Init();
	USART_Init();
	
	// Enable sleeping
	set_sleep_mode(0);
	sleep_enable();
	sei();
	
	while (1) {
		sleep_cpu(); // Polling is for losers
	}
}

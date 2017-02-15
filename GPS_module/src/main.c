/**
 * \file
 *
 * \brief Empty user application template
 *
 */

#define F_CPU 11059200UL // IMPORTANT: Replace this with frequency of occilator in final version

/* The largest size NMEA sentence that should be storable */
#define NMEA_BUFFER_SIZE 82

#include <asf.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <string.h>

int transmit_lock = 0; // Prevents the current buffered data from being replaced with new data
int is_reading_sentence = 0; // 1 if buffer is currently reading a valid NMEA sentence

char buffer1[NMEA_BUFFER_SIZE];
char* stored_ptr = buffer1;
char buffer2[NMEA_BUFFER_SIZE];
char* receiving_ptr = buffer2;
int receiving_size;

static void USART_Init(void){
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
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8 data, 1 stop bit */
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
	
	/* Release reset on GPS */
	PORTB |= 0x01;
}

unsigned int USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	
	/* Store received character */
	char msg = UDR0;
	
	/* Check for start of sentence */
	if (msg == '$') {
		/* Don't overwrite stored buffer if we are currently transmitting it */
		if (!transmit_lock) {
			// Only replace content of the stored buffer if the new data is a GPS fix sentence
			if (strncmp(receiving_ptr, "$GPGGA", 6) == 0) {
				char* old_stored = stored_ptr;
				stored_ptr = receiving_ptr;
				receiving_ptr = old_stored;
				
				volatile char tmp = *(stored_ptr+43);
				// Update GPS fix indication LED (Prototype only)
				if(*(stored_ptr+43) != '0') {
					PORTB |= 0x02;
				} else {
					PORTB &= ~(1<<DDB1);
				}
			}

			
			receiving_size = 0;
			
		}
	}
	if (receiving_size != NMEA_BUFFER_SIZE) {
		/* Put new character at the correct offset from the receive buffer */
		*(receiving_ptr+receiving_size) = msg;
		receiving_size++;
	}


	/* Get and return received data from buffer */
	return UDR0;
}

int main (void)
{
	/* Set PB0 to output (Use for reset signal on GPS) */
	DDRB = (1<<DDB0);
	
	/* Set PB1 to output (Use for fix indication LED in prototype) */
	DDRB = (1<<DDB1);
	PORTB |= 0x02;

	
	USART_Init();
	
	while (1) {
		USART_Receive();
	}


	/* Insert application code here, after the board has been initialized. */
}

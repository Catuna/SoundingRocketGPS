/**
 * \file
 *
 * \brief Empty user application template
 *
 */

#define F_CPU 11059200UL // IMPORTANT: Replace this with frequency of occilator in final version


/* The largest size NMEA sentence that should be storable */
#define NMEA_BUFFER_SIZE 82

#define OUTPUT_PORT PORTD //TODO: Not sure which output reg to use
#define LENGTH_OF_PARSED 26 // Length of parsed char array


#include <asf.h>
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "inttypes.h"
#include <stdlib.h>

int is_reading_sentence = 0; // 1 if buffer is currently reading a valid NMEA sentence

// Store buffer has the newest complete GPGGA message
char buffer1[82] = "$GPGGA,777777.777,7777.7777,N,77777.7777,W,7,7,7.77,77.7,M,77.7,M,,*77";
char* stored_ptr = buffer1;
uint8_t stored_size = 70;

// Receive buffer stores the incoming NMEA message
char buffer2[NMEA_BUFFER_SIZE];
char* receiving_ptr = buffer2;
uint8_t receiving_size;

uint8_t fields_of_interest[] = 	{ 2,4,6,7,9 };
uint8_t field_sizes[] = 		{ 8,9,1,2,6 };


//Function declarations
void set_output_pins(uint8_t value);
const char *byte_to_binary(int x);
void truncate_char_array(char *input, uint8_t length_of_input, uint8_t *output);
void parser(char *input, uint8_t length_of_input, char *output, uint8_t length_of_output);

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

ISR(UART0_RX_vect) {
    /* Store received character */
    char msg = UDR0;

    /* Check for start of sentence */
    if (msg == '$') {
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
    if (receiving_size != NMEA_BUFFER_SIZE) {
        /* Put new character at the correct offset from the receive buffer */
        *(receiving_ptr+receiving_size) = msg;
        receiving_size++;
    }
}

volatile uint8_t next_byte_to_send = 0;
volatile uint8_t array_currently_being_sent[13];

ISR(PCINT0_vect) {
	if (next_byte_to_send >= 13) {
		char parsed[LENGTH_OF_PARSED] = { '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0' };
		parse_NMEA(stored_ptr, stored_size, parsed, LENGTH_OF_PARSED);
		truncate_char_array(parsed, LENGTH_OF_PARSED, array_currently_being_sent);
		set_output_pins(0);
		next_byte_to_send = 0;
	}
	else {
		set_output_pins(array_currently_being_sent[next_byte_to_send]);
		next_byte_to_send++;
	}
}


//TODO: Rearrange the bit-pin config.
void set_output_pins(uint8_t value) {
	// Bits are rearranged to match the rocket controller pinout
	uint8_t data7 = (value & (1 << 7)) >> 3;
	uint8_t data6 = (value & (1 << 6)) >> 3;
	uint8_t data5 = (value & (1 << 5)) >> 0;
	uint8_t data4 = (value & (1 << 4)) >> 2;
	uint8_t data3 = (value & (1 << 3)) << 3;	// Notice: Left shift
	uint8_t data2 = (value & (1 << 2)) >> 1;
	uint8_t data1 = (value & (1 << 1)) << 6;	// Notice: Left shift
	uint8_t data0 = (value & (1 << 0)) >> 0;

	// Bits are assembled into a byte and put on output pins
	OUTPUT_PORT = data7 | data6 | data5 | data4 | data3 | data2 | data1 | data0;
}

//Extracts the fields of interests.
void parse_NMEA(char *input, uint8_t length_of_input, char *output, uint8_t length_of_output) {
	uint8_t j = 4; //iterate over fields of interest
	int8_t k = 25; //iterate over the output variable
	uint8_t current_field = 14; // Iterating from the end.  TODO: in example there are 14 fields and not 11 as in doc. check reality.
	uint8_t remaining_field_chars = 0;


	for (int i = length_of_input - 1; i >= 0; i--) {
		if (input[i] == ',') {
			current_field--;
			if (current_field < fields_of_interest[0]) {
				break;
			}
			else if (current_field == fields_of_interest[j]) {
				remaining_field_chars = field_sizes[j];
				j--;
			}
			continue; // Don't want to add ',' to output array
		}

		if (remaining_field_chars > 0) {
			if (remaining_field_chars > 1 && input[i - 1] == ',') {
				//we need to zero pad. All the fields are already '0', so we just need to jump:
				output[k] = input[i];
				k--;
				remaining_field_chars--;
				k -= remaining_field_chars;
				remaining_field_chars = 0;
			}
			else if (input[i] != '.') {
				output[k] = input[i];
				remaining_field_chars--;
				k--;
			}
		}
	}
}


void truncate_char_array(char *input, uint8_t length_of_input, uint8_t *output) {

	if (length_of_input % 2 != 0) { //If it is an odd number of elements
		output[length_of_input / 2] = (((uint8_t)input[length_of_input - 1]) - 48) << 4;
	}

	uint8_t j = 0;
	for (int i = 0; i < length_of_input - 1; i += 2) {
		output[j++] = ((((uint8_t)input[i]) - 48) << 4) | ((((uint8_t)input[i + 1]) - 48));
	}
}



int main (void)
{
	/* Set PB0 to output (Use for reset signal on GPS) */
	DDRB = (1<<DDB0);

	/* Set PB1 to output (Use for fix indication LED in prototype) */
	DDRB = (1<<DDB1);
	PORTB |= 0x02;


	USART_Init();

	while (1) {} // Polling is for losers
}

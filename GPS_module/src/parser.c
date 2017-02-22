#include "parser.h"

uint8_t fields_of_interest[] = 	{ 2,4,6,7,9 };
uint8_t field_sizes[] = 		{ 8,9,1,2,6 };

//TODO: Rearrange the bit-pin config.
char get_output_data(uint8_t value) {
	// Bits are rearranged to match the rocket controller pinout
	/*
	uint8_t data7 = (value & (1 << 7)) << 0; // MSB
	uint8_t data6 = (value & (1 << 6)) << 0;
	uint8_t data5 = (value & (1 << 5)) << 0;
	uint8_t data4 = (value & (1 << 4)) << 0;
	uint8_t data3 = (value & (1 << 3)) << 0;
	uint8_t data2 = (value & (1 << 2)) << 0;
	uint8_t data1 = (value & (1 << 1)) << 0;
	uint8_t data0 = (value & (1 << 0)) << 0; // LSB
	*/

	// Bits are assembled into a byte and put on output pins
	return value;
}

//Extracts the fields of interests.
void parse_NMEA(char *input, uint8_t length_of_input, char *output) {
	uint8_t j = 4; //iterate over fields of interest
	int8_t k = 25; //iterate over the output variable
	uint8_t current_field = 14; // Iterating from the end.  TODO: in example there are 14 fields and not 11 as in doc. check reality.
	uint8_t remaining_field_chars = 0;


	for (int i = length_of_input - 1; i >= 0; i--) {
		// a ',' is used to separate the fields of an NMEA message.
		if (input[i] == ',') {
			current_field--;
			if (current_field < fields_of_interest[0]) {
				break; // We have found all the fields of interest.
			}
			else if (current_field == fields_of_interest[j]) {
				remaining_field_chars = field_sizes[j];
				j--;
			}
			continue; // Don't want to add ',' to output array
		}

		if (remaining_field_chars > 0) {
			// Check if zero-padding is needed.
			if (remaining_field_chars > 1 && input[i - 1] == ',') {
				output[k] = input[i];
				k--;
				remaining_field_chars--;
				//All the fields are already '0', so we just need to jump:
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

// 15 indicates an error.
uint8_t validateValue(int a){
	if (a < 0 || a > 9){
		return 15;	
	}
	return ((uint8_t) a);
}

void truncate_char_array(char *input, uint8_t length_of_input, uint8_t *output) {
	
	// Used to separate the messages.
	output[0] = 0xff;
	output[1] = 0xff;

	if (length_of_input % 2 != 0) { //If it is an odd number of elements
		output[2 + length_of_input/2] = validateValue((uint8_t)input[length_of_input - 1] - 48) << 4;
	}

	uint8_t j = 0; //Used to iterate over the output array.
	for (int i = 0; i < length_of_input - 1; i += 2) {
		output[2+j++] = (validateValue( (uint8_t)input[i]-48 )  << 4) | validateValue( (uint8_t)input[i + 1]-48 );
	}

}
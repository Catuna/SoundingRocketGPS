#include "parser.h"

uint8_t fields_of_interest[] = 	{ 2,4,6,7,9 };
uint8_t field_sizes[] = 		{ 8,9,1,2,6 };

//TODO: Rearrange the bit-pin config.
char get_output_data(uint8_t value) {
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
	return data7 | data6 | data5 | data4 | data3 | data2 | data1 | data0;
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
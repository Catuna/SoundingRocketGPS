#ifndef PARSER_H
#define PARSER_H

#include <inttypes.h>

char get_output_data(uint8_t value);

const char *byte_to_binary(int x); //TODO: Unused?

void truncate_char_array(char *input, uint8_t length_of_input, uint8_t *output);

void parse_NMEA(char *input, uint8_t length_of_input, char *output);

#endif
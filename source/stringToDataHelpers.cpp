#include <string>

#include "stringToDataHelpers.h"

void copy_string_to_sprite_memory(uint8_t sprite_data[128 * 64], std::string data) {
	uint16_t i = 0;

	for (size_t n = 0; n < data.length(); n++) {
		char buf[3] = {0};

		//https://pico-8.fandom.com/wiki/Memory
		// "An 8-bit byte represents two pixels, horizontally adjacent, where the most significant 
		// (leftmost) 4 bits is the right pixel of the pair, and the least significant 4 bits is 
		// the left pixel.
		if (data[n] > ' ') {
			//buf[0] = data[n++];
			//buf[1] = data[n];
			buf[1] = data[n++]; //left pixel 
			buf[0] = data[n];
			uint8_t val = (uint8_t)strtol(buf, NULL, 16);

			//this was the impl before combining nibbles into a single byte
			//sprite_data[i++] = val >> 4;
			//sprite_data[i++] = val & 0x0f;
			sprite_data[i++] = val;
		}
	}
}

void copy_string_to_memory(uint8_t* sprite_flag_data, std::string data) {
	uint16_t i = 0;

	for (size_t n = 0; n < data.length(); n++) {
		char buf[3] = {0};

		if (data[n] > ' ') {
			buf[0] = data[n++];
			buf[1] = data[n];
			uint8_t val = (uint8_t)strtol(buf, NULL, 16);

			sprite_flag_data[i++] = val;
		}
	}
}
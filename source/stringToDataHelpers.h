#pragma once

#include <string>

void copy_string_to_sprite_memory(uint8_t sprite_data[128 * 64], std::string data);

void copy_string_to_memory(uint8_t* sprite_flag_data, std::string data);
#pragma once

#include <string>

void copy_data_to_sprites(uint8_t sprite_data[128 * 64], std::string data);

void copy_data_to_sprite_flags(uint8_t sprite_flag_data[256], std::string data);
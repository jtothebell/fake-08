#pragma once

#include <stdint.h>
#include <string>

#include <string>

/// Creates a bitmask from a bit number.
#define BITMASK(n) (1U<<(n))

#define COLOR_00 {  2,   4,   8, 255}
#define COLOR_01 { 29,  43,  83, 255}
#define COLOR_02 {126,  37,  83, 255}
#define COLOR_03 {  0, 135,  81, 255}
#define COLOR_04 {171,  82,  54, 255}
#define COLOR_05 { 95,  87,  79, 255}
#define COLOR_06 {194, 195, 199, 255}
#define COLOR_07 {255, 241, 232, 255}
#define COLOR_08 {255,   0,  77, 255}
#define COLOR_09 {255, 163,   0, 255}
#define COLOR_10 {255, 236,  39, 255}
#define COLOR_11 {  0, 228,  54, 255}
#define COLOR_12 { 41, 173, 255, 255}
#define COLOR_13 {131, 118, 156, 255}
#define COLOR_14 {255, 119, 168, 255}
#define COLOR_15 {255, 204, 170, 255}

//alt palette
#define COLOR_128 { 41,  24,  20, 255}
#define COLOR_129 { 17,  29,  53, 255}
#define COLOR_130 { 66,  33,  54, 255}
#define COLOR_131 { 18,  83,  89, 255}
#define COLOR_132 {116,  47,  41, 255}
#define COLOR_133 { 73,  51,  59, 255}
#define COLOR_134 {162, 136, 121, 255}
#define COLOR_135 {243, 239, 125, 255}
#define COLOR_136 {190,  18,  80, 255}
#define COLOR_137 {255, 108,  36, 255}
#define COLOR_138 {168, 231,  46, 255}
#define COLOR_139 {  0, 181,  67, 255}
#define COLOR_140 {  6,  90, 181, 255}
#define COLOR_141 {117,  70, 101, 255}
#define COLOR_142 {255, 110,  89, 255}
#define COLOR_143 {255, 157, 129, 255}

enum
{
	P8_KEY_LEFT   = BITMASK(0),
	P8_KEY_RIGHT  = BITMASK(1),
	P8_KEY_UP     = BITMASK(2),
	P8_KEY_DOWN   = BITMASK(3),
	P8_KEY_O      = BITMASK(4),
	P8_KEY_X      = BITMASK(5),
	P8_KEY_PAUSE  = BITMASK(6),
	P8_KEY_7      = BITMASK(7),
};

struct Color {
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
	uint8_t Alpha;
};

struct Bgr24Col {
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
};

struct InputState_t {
	uint8_t KDown;
	uint8_t KHeld;

	int16_t mouseX;
	int16_t mouseY;
	uint8_t mouseBtnState;
	bool KBdown;
	std::string KBkey;
};

enum PrintMode_t
{
	PRINT_MODE_ON = 0x1,
    PRINT_MODE_PADDING = 0x2,
	PRINT_MODE_WIDE = 0x4,
    PRINT_MODE_TALL = 0x8,
	PRINT_MODE_SOLID_BG = 0x10,
	PRINT_MODE_INVERTED = 0x20,
	PRINT_MODE_STRIPEY = 0x40,
	PRINT_MODE_CUSTOM_FONT = 0x80
};

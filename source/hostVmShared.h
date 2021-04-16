#pragma once

/// Creates a bitmask from a bit number.
#define BITMASK(n) (1U<<(n))

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
};

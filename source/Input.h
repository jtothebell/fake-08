#pragma once

#include <stdint.h>

class Input{
	uint8_t _currentKDown;
	uint8_t _currentKHeld;

	uint16_t _framesHeld[8];
	
    public:
    void SetState(uint8_t kdown, uint8_t kheld);
    

	uint8_t btn();
    uint8_t btnp();

    bool btn(uint8_t i);
    bool btnp(uint8_t i);

	bool btn(uint8_t i, uint8_t p);
    bool btnp(uint8_t i, uint8_t p);
};
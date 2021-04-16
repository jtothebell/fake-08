#pragma once

#include <stdint.h>
#include "PicoRam.h"

class Input{
    PicoRam* _memory;
	uint8_t _currentKDown;

	uint16_t _framesHeld[8];

    int16_t _mouseX;
    int16_t _mouseY;
    uint8_t _mouseBtnState;
	
    public:
    Input(PicoRam* memory);
    void SetState(uint8_t kdown, uint8_t kheld);
    void SetMouse(int16_t mouseX, int16_t mouseY, uint8_t mouseBtnState);

	uint8_t btn();
    uint8_t btnp();

    bool btn(uint8_t i);
    bool btnp(uint8_t i);

	bool btn(uint8_t i, uint8_t p);
    bool btnp(uint8_t i, uint8_t p);

    int16_t getMouseX();
    int16_t getMouseY();
    uint8_t getMouseBtnState();
};
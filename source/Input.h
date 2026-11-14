#pragma once

#include <stdint.h>
#include "PicoRam.h"

class Input{
    PicoRam* _memory;
	uint8_t _currentKDown[8];
    uint8_t _currentKHeld[8];

	// frames each button of each player has been held, in calls to SetState
	uint16_t _framesHeld[8][8];
	// calls to SetState per 30 fps frame: 2 when it runs at 60 Hz
	uint8_t _frameScale;

    int16_t _mouseX;
    int16_t _mouseY;
    uint8_t _mouseBtnState;
	bool _kbDown;
	std::string _kbKey;
	
    public:
    Input(PicoRam* memory);
    void SetState(uint8_t kdown, uint8_t kheld);
    // every player at once: player 0 first
    void SetState(const uint8_t* kdown, const uint8_t* kheld, int players);
    // 2 when SetState runs at 60 Hz (an _update60 cart, the pause menu)
    void SetFrameScale(uint8_t scale);
    void SetMouse(int16_t mouseX, int16_t mouseY, uint8_t mouseBtnState);
	
    void SetKeyboard(bool kbdown, std::string kbkey);

	uint16_t btn();
    uint16_t btnp();

    bool btn(uint8_t i);
    bool btnp(uint8_t i);

	bool btn(uint8_t i, uint8_t p);
    bool btnp(uint8_t i, uint8_t p);

    int16_t getMouseX();
    int16_t getMouseY();
    uint8_t getMouseBtnState();
	
    bool getKeyDown();
	const char* getKey();
};
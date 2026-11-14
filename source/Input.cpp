#include <algorithm>

#include "Input.h"
#include "hostVmShared.h"
#include "PicoRam.h"

Input::Input(PicoRam* memory):
    _frameScale(1)
{
    _memory = memory;

    std::fill(&_currentKDown[0], &_currentKDown[0] + 8, 0);
    std::fill(&_currentKHeld[0], &_currentKHeld[0] + 8, 0);
    std::fill(&_framesHeld[0][0], &_framesHeld[0][0] + 64, 0);
}

void Input::SetFrameScale(uint8_t scale){
    _frameScale = scale == 0 ? 1 : scale;
}

void Input::SetState(uint8_t kdown, uint8_t kheld){
    uint8_t down[8] = {kdown};
    uint8_t held[8] = {kheld};
    SetState(down, held, 8);
}

void Input::SetState(const uint8_t* kdown, const uint8_t* kheld, int players){
    // btnp() repeat, from the PICO-8 manual: held for 15 frames it counts as
    // pressed again, then every 4 frames. Custom delays (0x5f5c, 0x5f5d) are
    // in frames at 30 fps, 255 turns repeat off. SetState runs once per
    // 30 fps frame or twice (_frameScale) at 60 Hz, so the counts scale.
    uint8_t delayFrames = _memory->hwState.btnpRepeatDelay == 0
        ? 15
        : _memory->hwState.btnpRepeatDelay;
    uint8_t intervalFrames = _memory->hwState.btnpRepeatInterval == 0
        ? 4
        : _memory->hwState.btnpRepeatInterval;
    bool repeat = delayFrames != 255;
    uint16_t delay = delayFrames * _frameScale;
    uint16_t interval = intervalFrames * _frameScale;

    for (int p = 0; p < 8; p++) {
        uint8_t down = p < players ? kdown[p] : 0;
        uint8_t held = p < players ? kheld[p] : 0;
        if (p == 0) {
            //key 6 (PAUSE MENU) only fires for one frame, even if held
            if ((held & BITMASK(6)) && !(down & BITMASK(6))) {
                held = held & ~(BITMASK(6));
            }
        }
        _currentKHeld[p] = held;
        //memory only stores buttons 0-5, not 6 or 7
        _memory->hwState.buttonStates[p] = held & 0x3F;

        for (int i = 0; i < 7; i++) {
            bool isHeld = BITMASK(i) & held;
            _framesHeld[p][i] = isHeld ? _framesHeld[p][i] + 1 : 0;
            uint16_t f = _framesHeld[p][i];
            if (repeat && (f == delay || (f > delay && (f - delay) % interval == 0))) {
                down = down | BITMASK(i);
            }
        }
        _currentKDown[p] = down;
    }
}

void Input::SetMouse(int16_t mouseX, int16_t mouseY, uint8_t mouseBtnState){
    _mouseX = mouseX;
    _mouseY = mouseY;
    _mouseBtnState = mouseBtnState;
}

void Input::SetKeyboard(bool kbDown, std::string kbKey){
	_kbDown = kbDown;
	_kbKey = kbKey;
}

// PICO-8: player 0 in bits 0-5 (and pause in 6), player 1 in bits 8-13
uint16_t Input::btn(){
    return _currentKHeld[0] | ((_currentKHeld[1] & 0x3F) << 8);
}

uint16_t Input::btnp(){
    return _currentKDown[0] | ((_currentKDown[1] & 0x3F) << 8);
}

bool Input::btn(uint8_t i){
    return btn(i, 0);
}

bool Input::btnp(uint8_t i){
    return btnp(i, 0);
}

bool Input::btn(uint8_t i, uint8_t p){
    return i < 8 && p < 8 && (BITMASK(i) & _currentKHeld[p]);
}

bool Input::btnp(uint8_t i, uint8_t p){
    return i < 8 && p < 8 && (BITMASK(i) & _currentKDown[p]);
}

int16_t Input::getMouseX() {
    return _mouseX;
}

int16_t Input::getMouseY() {
    return _mouseY;
}

uint8_t Input::getMouseBtnState() {
    return _mouseBtnState;
}

bool Input::getKeyDown() {
    return _kbDown;
}

const char* Input::getKey() {
    // Found this bug in Terra
    // PICO-8 behavior: stat(31) appears to consume the key from the buffer
    // After reading, stat(30) should return false until next key press
    _kbDown = false;
    return _kbKey.c_str();
}


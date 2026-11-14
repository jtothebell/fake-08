#include <algorithm>

#include "Input.h"
#include "hostVmShared.h"
#include "PicoRam.h"

Input::Input(PicoRam* memory):
    _currentKDown(0),
    _currentKHeld(0),
    _frameScale(1)
{
    _memory = memory;

    std::fill(_framesHeld, _framesHeld + 8, 0);
}

void Input::SetFrameScale(uint8_t scale){
    _frameScale = scale == 0 ? 1 : scale;
}

void Input::SetState(uint8_t kdown, uint8_t kheld){
    _currentKDown = kdown;
    _currentKHeld = kheld;
    //key 6 (PAUSE MENU) only fires for one frame, even if held
    if ((_currentKHeld & BITMASK(6)) && !(_currentKDown & BITMASK(6))) {
        _currentKHeld = _currentKHeld & ~(BITMASK(6));
    }
    //memory only stores buttons 0-5, not 6 or 7
    _memory->hwState.buttonStates[0] = kheld & 0x3F;

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

    for (int i = 0; i < 7; i++) {
        bool isHeld = BITMASK(i) & kheld;
        _framesHeld[i] = isHeld ? _framesHeld[i] + 1 : 0;
        uint16_t f = _framesHeld[i];
        if (repeat && (f == delay || (f > delay && (f - delay) % interval == 0))) {
            _currentKDown = _currentKDown | BITMASK(i);
        }
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

uint8_t Input::btn(){
    return _currentKHeld;
}

//todo: repetition behavior to match pico 8
uint8_t Input::btnp(){
    return _currentKDown;
}

bool Input::btn(uint8_t i){
    return BITMASK(i) & btn();
}

//todo: repetition behavior to match pico 8
bool Input::btnp(uint8_t i){
    return BITMASK(i) & btnp();
}

bool Input::btn(uint8_t i, uint8_t p){
    //no multiplayer support for now
    return p == 0 ? btn(i) : 0;
}

//todo: repetition behavior to match pico 8
bool Input::btnp(uint8_t i, uint8_t p){
    //no multiplayer support for now
    return p == 0 ? btnp(i) : 0;
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


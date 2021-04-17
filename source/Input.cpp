#include <algorithm>

#include "Input.h"
#include "hostVmShared.h"
#include "PicoRam.h"

Input::Input(PicoRam* memory):
    _currentKDown(0)
{
    _memory = memory;

    std::fill(_framesHeld, _framesHeld + 8, 0);
}

void Input::SetState(uint8_t kdown, uint8_t kheld){
    _currentKDown = kdown;
    _memory->hwState.buttonStates[0] = kheld;

    uint8_t repeatDelay = _memory->hwState.btnpRepeatDelay == 0 
        ? 15 
        : _memory->hwState.btnpRepeatDelay;

    if (repeatDelay == 255){
        return;
    }

    uint8_t repeatInterval = _memory->hwState.btnpRepeatInterval == 0 
        ? 4 
        : _memory->hwState.btnpRepeatDelay;

    for (int i = 0; i < 8; i ++) {
        bool down = BITMASK(i) & kheld;

        _framesHeld[i] = down ? _framesHeld[i] + 1 : 0;

        //update kdown to be true if held for 15 frames, then every 4th after that.
        //from wiki:
        //btnp() implements a keyboard-like repeat mechanism: if the player holds 
        //the button for 15 frames, it registers as on again for one frame, then 
        //again every four frames after that. The frame counter resets when the 
        //player releases the button. 

        bool repeatPressed = 
            (_framesHeld[i] == repeatDelay) ||
            (_framesHeld[i] / repeatDelay >= 1 && _framesHeld[i] % repeatInterval == 0);

        if (repeatPressed) {
            _currentKDown = _currentKDown | BITMASK(i);
        }
    }
}

void Input::SetMouse(int16_t mouseX, int16_t mouseY, uint8_t mouseBtnState){
    _mouseX = mouseX;
    _mouseY = mouseY;
    _mouseBtnState = mouseBtnState;
}

uint8_t Input::btn(){
    return _memory->hwState.buttonStates[0];
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

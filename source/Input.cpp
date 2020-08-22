#include "Input.h"
#include "hostVmShared.h"

void Input::SetState(uint8_t kdown, uint8_t kheld){
    _currentKDown = kdown;
    _currentKHeld = kheld;

    //TODO: this can be overridden by poking certain memory address
    for (int i = 0; i < 8; i ++) {
        bool down = BITMASK(i) & _currentKHeld;

        _framesHeld[i] = down ? _framesHeld[i] + 1 : 0;

        //update kdown to be true if held for 15 frames, then every 4th after that.
        //from wiki:
        //btnp() implements a keyboard-like repeat mechanism: if the player holds 
        //the button for 15 frames, it registers as on again for one frame, then 
        //again every four frames after that. The frame counter resets when the 
        //player releases the button. 

        bool repeatPressed = 
            (_framesHeld[i] == 15) ||
            (_framesHeld[i] / 15 >= 1 && _framesHeld[i] % 4 == 0);

        if (repeatPressed) {
            _currentKDown = _currentKDown | BITMASK(i);
        }
    }
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
    return p == 0 ? btn(i) : 0;
}

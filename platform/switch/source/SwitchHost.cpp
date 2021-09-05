#include <switch.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "../../SDL2Common/source/sdl2basehost.h"

#include "../../../source/host.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"
#include "../../../source/logger.h"
#include "../../../source/filehelpers.h"

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#define WINDOW_SIZE_X 1280
#define WINDOW_SIZE_Y 720

#define WINDOW_FLAGS SDL_WINDOW_SHOWN

#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED
#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888


string _desktopSdl2SettingsDir = "switch/fake08";
string _desktopSdl2SettingsPrefix = "switch/fake08/";
string _switchCartDir = "/p8carts";
string _desktopSdl2customBiosLua = "cartpath = \"sdmc:/p8carts/\"\n";

PadState pad;
u64 currKDown_64;
u64 currKHeld_64;

int touchLocationX;
int touchLocationY;
uint8_t mouseBtnState;

uint8_t ConvertInputToP8(u64 input){
	uint8_t result = 0;
	if (input & HidNpadButton_Left){
		result |= P8_KEY_LEFT;
	}

	if (input & HidNpadButton_Right){
		result |= P8_KEY_RIGHT;
	}

	if (input & HidNpadButton_Up){
		result |= P8_KEY_UP;
	}

	if (input & HidNpadButton_Down){
		result |= P8_KEY_DOWN;
	}

	if (input & HidNpadButton_B){
		result |= P8_KEY_O;
	}

	if (input & HidNpadButton_A){
		result |= P8_KEY_X;
	}

	if (input & HidNpadButton_Plus){
		result |= P8_KEY_PAUSE;
	}

	if (input & HidNpadButton_Minus){
		result |= P8_KEY_7;
	}

	return result;
}

Host::Host() 
{
    struct stat st = {0};

    int res = chdir("sdmc:/");
    if (res == 0 && stat("switch", &st) == -1) {
        res = mkdir("switch", 0777);
    }

    if (res == 0 && stat(_desktopSdl2SettingsDir.c_str(), &st) == -1) {
        res = mkdir(_desktopSdl2SettingsDir.c_str(), 0777);
    }

    string cartdatadir = _desktopSdl2SettingsPrefix + "cdata";
    if (res == 0 && stat(cartdatadir.c_str(), &st) == -1) {
        res = mkdir(cartdatadir.c_str(), 0777);
    }

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    
    padInitializeDefault(&pad);

    setPlatformParams(
        WINDOW_SIZE_X,
        WINDOW_SIZE_Y,
        WINDOW_FLAGS,
        RENDERER_FLAGS,
        PIXEL_FORMAT,
        _desktopSdl2SettingsPrefix,
        _desktopSdl2customBiosLua,
        _switchCartDir
    );
}


InputState_t Host::scanInput(){
    //SDL input doesn't seem to work correctly on the switch, so I've left the switch specific one
    //the issue may be related to having multiple controllers?
    padUpdate(&pad);

    currKDown_64 = padGetButtonsDown(&pad);
    currKHeld_64 = padGetButtons(&pad);

    lDown = currKHeld_64 & HidNpadButton_L;
	rDown = currKDown_64 & HidNpadButton_R;

    if (lDown && rDown){
        quit = 1;
    }

    stretchKeyPressed = currKDown_64 & HidNpadButton_Minus;

    HidTouchScreenState state={0};
    if (hidGetTouchScreenStates(&state, 1)) {
        if (state.count >= 1){
            touchLocationX = (state.touches[0].x - mouseOffsetX) / scaleX;
            touchLocationY = (state.touches[0].y - mouseOffsetY) / scaleY;
            mouseBtnState = 1;
        }
        else{
            mouseBtnState = 0;
        }
    }

    return InputState_t {
        ConvertInputToP8(currKDown_64),
        ConvertInputToP8(currKHeld_64),
        (int16_t)touchLocationX,
        (int16_t)touchLocationY,
        mouseBtnState
    };
}


vector<string> Host::listcarts(){
    vector<string> carts;

    DIR* dir = opendir(_cartDirectory.c_str());
    struct dirent *ent;

    if (dir) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name)){
                carts.push_back(_cartDirectory + "/" + ent->d_name);
            }
        }
        closedir (dir);
    }

    
    return carts;
}


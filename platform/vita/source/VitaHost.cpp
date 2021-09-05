
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "../../SDL2Common/source/sdl2basehost.h"

#include "../../../source/host.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"
#include "../../../source/logger.h"
#include "../../../source/filehelpers.h"

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>
#include <psp2/power.h>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#define WINDOW_SIZE_X 960
#define WINDOW_SIZE_Y 544

#define R_SUCCEEDED(res)   ((res)>=0)
#define R_FAILED(res)      ((res)<0)

#define SDLK_VITA_TRIANGLE 0
#define SDLK_VITA_CIRCLE 1 
#define SDLK_VITA_CROSS 2
#define SDLK_VITA_SQUARE 3
#define SDLK_VITA_LTRIGGER 4
#define SDLK_VITA_RTRIGGER 5
#define SDLK_VITA_DOWN 6
#define SDLK_VITA_LEFT 7
#define SDLK_VITA_UP 8
#define SDLK_VITA_RIGHT 9
#define SDLK_VITA_SELECT 10
#define SDLK_VITA_START 11

#define WINDOW_FLAGS SDL_WINDOW_FULLSCREEN_DESKTOP

#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED
#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888


//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;
int jxDir = 0;
int jyDir = 0;

SDL_Event event;
SDL_Point touchLocation = { 128 / 2, 128 / 2 };


string _desktopSdl2SettingsDir = "ux0:/data/fake08";
string _desktopSdl2SettingsPrefix = "ux0:/data/fake08/";
string _vitaCartDir = "ux0:/p8carts";
string _desktopSdl2customBiosLua = "cartpath = \"ux0:/p8carts/\"\n"
        "selectbtn = \"x\"\n"
        "pausebtn = \"start\"";


Host::Host() {
    //make sure directories exists
    sceIoMkdir("ux0:/data/", 0777);
    sceIoMkdir("ux0:/data/fake08/", 0777);
    sceIoMkdir("ux0:/data/fake08/cdata", 0777);

    scePowerSetArmClockFrequency( 444 );
    scePowerSetBusClockFrequency( 222 );
    scePowerSetGpuClockFrequency( 222 );
    scePowerSetGpuXbarClockFrequency( 166 );

    setPlatformParams(
        WINDOW_SIZE_X,
        WINDOW_SIZE_Y,
        WINDOW_FLAGS,
        RENDERER_FLAGS,
        PIXEL_FORMAT,
        _desktopSdl2SettingsPrefix,
        _desktopSdl2customBiosLua,
        _vitaCartDir
    );
}

InputState_t Host::scanInput(){ 
    currKDown = 0;
    uint8_t kUp = 0;
    int prevJxDir = jxDir;
    int prevJyDir = jyDir;
    stretchKeyPressed = false;
    
    uint8_t mouseBtnState = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYBUTTONDOWN :
                switch (event.jbutton.button)
                {
                    case SDLK_VITA_START:currKDown |= P8_KEY_PAUSE; break;
                    case SDLK_VITA_LEFT:  currKDown |= P8_KEY_LEFT; break;
                    case SDLK_VITA_RIGHT: currKDown |= P8_KEY_RIGHT; break;
                    case SDLK_VITA_UP:    currKDown |= P8_KEY_UP; break;
                    case SDLK_VITA_DOWN:  currKDown |= P8_KEY_DOWN; break;
                    case SDLK_VITA_CROSS:     currKDown |= P8_KEY_X; break;
                    case SDLK_VITA_CIRCLE:     currKDown |= P8_KEY_O; break;
                    case SDLK_VITA_LTRIGGER: lDown = true; break;
                    case SDLK_VITA_RTRIGGER: rDown = true; break;
                    case SDLK_VITA_SELECT: stretchKeyPressed = true; break;
                }
                break;

            case SDL_JOYBUTTONUP :
                switch (event.jbutton.button)
                {
                    case SDLK_VITA_START: kUp |= P8_KEY_PAUSE; break;
                    case SDLK_VITA_LEFT:  kUp |= P8_KEY_LEFT; break;
                    case SDLK_VITA_RIGHT: kUp |= P8_KEY_RIGHT; break;
                    case SDLK_VITA_UP:    kUp |= P8_KEY_UP; break;
                    case SDLK_VITA_DOWN:  kUp |= P8_KEY_DOWN; break;
                    case SDLK_VITA_CROSS:     kUp |= P8_KEY_X; break;
                    case SDLK_VITA_CIRCLE:     kUp |= P8_KEY_O; break;
                    case SDLK_VITA_LTRIGGER: lDown = false; break;
                    case SDLK_VITA_RTRIGGER: rDown = false; break;
                }
               break;

            case SDL_JOYAXISMOTION :
                if (event.jaxis.which == 0)
                {
                    //X axis motion
                    if( event.jaxis.axis == 0 )
                    {
                        //Left of dead zone
                        if( event.jaxis.value < -JOYSTICK_DEAD_ZONE )
                        {
                            jxDir = -1;
                        }
                        //Right of dead zone
                        else if( event.jaxis.value > JOYSTICK_DEAD_ZONE )
                        {
                            jxDir =  1;
                        }
                        else
                        {
                            jxDir = 0;
                        }
                    }
                    //Y axis motion
                    else if( event.jaxis.axis == 1 )
                    {
                        //Below of dead zone
                        if( event.jaxis.value < -JOYSTICK_DEAD_ZONE )
                        {
                            jyDir = -1;
                        }
                        //Above of dead zone
                        else if( event.jaxis.value > JOYSTICK_DEAD_ZONE )
                        {
                            jyDir =  1;
                        }
                        else
                        {
                            jyDir = 0;
                        }
                    }
                }
               break;

            case SDL_FINGERDOWN:
                //touchId 0 is front, 1 is back. ignore back touches
                if (event.tfinger.touchId == 0) {
                    touchLocation.x = ((event.tfinger.x * WINDOW_SIZE_X) - mouseOffsetX) / scaleX;
                    touchLocation.y = ((event.tfinger.y * WINDOW_SIZE_Y) - mouseOffsetY) / scaleY;
                    mouseBtnState = 1;
                }
                break;
            
            case SDL_FINGERMOTION:
                //touchId 0 is front, 1 is back. ignore back touches
                if (event.tfinger.touchId == 0) {
                    touchLocation.x = ((event.tfinger.x * WINDOW_SIZE_X) - mouseOffsetX) / scaleX;
                    touchLocation.y = ((event.tfinger.y * WINDOW_SIZE_Y) - mouseOffsetY) / scaleY;
                    mouseBtnState = 1;
                }
                break;

            case SDL_FINGERUP:
                //do nothing for now?
                mouseBtnState = 0;
                break;

            case SDL_QUIT:
                quit = 1;
                break;
        }
    }

    if (lDown && rDown){
        quit = 1;
    }

    currKHeld |= currKDown;
    currKHeld ^= kUp;


    //Convert joystick direction to kHeld and kDown values
    if (jxDir > 0) {
        currKHeld |= P8_KEY_RIGHT;
        currKHeld &= ~(P8_KEY_LEFT);

        if (prevJxDir != jxDir){
            currKDown |= P8_KEY_RIGHT;
        }
    }
    else if (jxDir < 0) {
        currKHeld |= P8_KEY_LEFT;
        currKHeld &= ~(P8_KEY_RIGHT);

        if (prevJxDir != jxDir){
            currKDown |= P8_KEY_LEFT;
        }
    }
    else if (prevJxDir != 0){
        currKHeld &= ~(P8_KEY_RIGHT);
        currKHeld &= ~(P8_KEY_LEFT);
        currKDown &= ~(P8_KEY_RIGHT);
        currKDown &= ~(P8_KEY_LEFT);
    }
    
    if (jyDir > 0) {
        currKHeld |= P8_KEY_DOWN;
        currKHeld &= ~(P8_KEY_UP);

        if (prevJyDir != jyDir){
            currKDown |= P8_KEY_DOWN;
        }
    }
    else if (jyDir < 0) {
        currKHeld |= P8_KEY_UP;
        currKHeld &= ~(P8_KEY_DOWN);

        if (prevJyDir != jyDir){
            currKDown |= P8_KEY_UP;
        }
    }
    else if (prevJyDir != 0){
        currKHeld &= ~(P8_KEY_UP);
        currKHeld &= ~(P8_KEY_DOWN);
        currKDown &= ~(P8_KEY_UP);
        currKDown &= ~(P8_KEY_DOWN);
    }
    


    return InputState_t {
        currKDown,
        currKHeld,
        (int16_t)touchLocation.x,
        (int16_t)touchLocation.y,
        mouseBtnState
    };
    
}


vector<string> Host::listcarts(){
    vector<string> carts;

    SceUID dir = 0;
    int ret = 0;

    if (R_SUCCEEDED(dir = sceIoDopen(_cartDirectory.c_str()))) {
        
        do {
            SceIoDirent dirent;

            if (R_FAILED(ret = sceIoDread(dir, &dirent))) {
                Logger_Write("Error: sceIoDread(%s) failed: 0x%lx\n", _cartDirectory.c_str(), ret);
                continue;
            }
            
            if (SCE_S_ISDIR(dirent.d_stat.st_mode)) {
                continue;
            }

            if (isCartFile(dirent.d_name)){
                carts.push_back(_cartDirectory + "/" + dirent.d_name);
            }
        } while (ret > 0);

        sceIoDclose(dir);
    }

    return carts;
}



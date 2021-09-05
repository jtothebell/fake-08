
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "../../SDL2Common/source/sdl2basehost.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"
#include "../../../source/filehelpers.h"
#include "../../../source/logger.h"

// sdl
#include <SDL2/SDL.h>

#define WINDOW_SIZE_X 1280
#define WINDOW_SIZE_Y 720

#define WINDOW_FLAGS 0

#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED
#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

SDL_Event event;


string _desktopSdl2SettingsDir = "fake08";
string _desktopSdl2SettingsPrefix = "fake08/";
string _desktopSdl2customBiosLua = "cartpath = \"~/p8carts/\"\n"
        "selectbtn = \"z\"\n"
        "pausebtn = \"esc\"\n"
        "exitbtn = \"close window\"\n"
        "sizebtn = \"\"";

Host::Host() 
{
    struct stat st = {0};

    int res = chdir(getenv("HOME"));
    if (res == 0 && stat(_desktopSdl2SettingsDir.c_str(), &st) == -1) {
        res = mkdir(_desktopSdl2SettingsDir.c_str(), 0777);
    }
    
    string cartdatadir = _desktopSdl2SettingsPrefix + "cdata";
    if (res == 0 && stat(cartdatadir.c_str(), &st) == -1) {
        res = mkdir(cartdatadir.c_str(), 0777);
    }

    std::string home = getenv("HOME");
    
    std::string fullCartDir = home + "/p8carts";

    setPlatformParams(
        WINDOW_SIZE_X,
        WINDOW_SIZE_Y,
        WINDOW_FLAGS,
        RENDERER_FLAGS,
        PIXEL_FORMAT,
        _desktopSdl2SettingsPrefix,
        _desktopSdl2customBiosLua,
        fullCartDir
    );
}


InputState_t Host::scanInput(){
    currKDown = 0;
    currKHeld = 0;
    stretchKeyPressed = false;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:currKDown |= P8_KEY_PAUSE; break;
                    case SDLK_LEFT:  currKDown |= P8_KEY_LEFT; break;
                    case SDLK_RIGHT: currKDown |= P8_KEY_RIGHT; break;
                    case SDLK_UP:    currKDown |= P8_KEY_UP; break;
                    case SDLK_DOWN:  currKDown |= P8_KEY_DOWN; break;
                    case SDLK_z:     currKDown |= P8_KEY_X; break;
                    case SDLK_x:     currKDown |= P8_KEY_O; break;
                    case SDLK_c:     currKDown |= P8_KEY_X; break;
                    case SDLK_r:     stretchKeyPressed = true; break;
                }
                break;

            case SDL_QUIT:
                quit = 1;
                break;
        }
    }

    int mouseX = 0;
	int mouseY = 0;
    uint32_t sdlMouseBtnState = SDL_GetMouseState(&mouseX, &mouseY);
    //adjust for scale
    mouseX -= mouseOffsetX;
    mouseY -= mouseOffsetY;
    mouseX /= scaleX;
    mouseY /= scaleY;
    uint8_t picoMouseState = 0;
    if (sdlMouseBtnState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        picoMouseState |= 1;
    }
    if (sdlMouseBtnState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) {
        picoMouseState |= 4;
    }
    if (sdlMouseBtnState & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
        picoMouseState |= 2;
    }

    const Uint8* keystate = SDL_GetKeyboardState(NULL);

    //continuous-response keys
    if(keystate[SDL_SCANCODE_LEFT]){
        currKHeld |= P8_KEY_LEFT;
    }
    if(keystate[SDL_SCANCODE_RIGHT]){
        currKHeld |= P8_KEY_RIGHT;;
    }
    if(keystate[SDL_SCANCODE_UP]){
        currKHeld |= P8_KEY_UP;
    }
    if(keystate[SDL_SCANCODE_DOWN]){
        currKHeld |= P8_KEY_DOWN;
    }
    if(keystate[SDL_SCANCODE_Z]){
        currKHeld |= P8_KEY_X;
    }
    if(keystate[SDL_SCANCODE_X]){
        currKHeld |= P8_KEY_O;
    }
    if(keystate[SDL_SCANCODE_C]){
        currKHeld |= P8_KEY_X;
    }
    
    return InputState_t {
        currKDown,
        currKHeld,
        (int16_t)mouseX,
        (int16_t)mouseY,
        picoMouseState
    };
}

vector<string> Host::listcarts(){
    vector<string> carts;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_cartDirectory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name)){
                carts.push_back(ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }
    
    return carts;
}


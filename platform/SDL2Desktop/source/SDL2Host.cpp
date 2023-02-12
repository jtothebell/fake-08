
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

#include "../../../source/emojiconversion.h"

// sdl
#include <SDL2/SDL.h>

/*#define WINDOW_SIZE_X 1280
#define WINDOW_SIZE_Y 720*/

#define WINDOW_FLAGS 0

#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED
#define PIXEL_FORMAT SDL_PIXELFORMAT_ARGB8888

#define KB_ENABLED true

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

    SDL_DisplayMode current;

    SDL_Init(SDL_INIT_VIDEO);

    int should_be_zero = SDL_GetCurrentDisplayMode(0, &current);

    if(should_be_zero != 0) {
      // In case of error...
      SDL_Log("Could not get display mode for video display #%d: %s", 0, SDL_GetError());
    }

    else {
      // On success, print the current display mode.
      SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", 0, current.w, current.h, current.refresh_rate);
    }

    int WINDOW_SIZE_X=current.w;
    int WINDOW_SIZE_Y=current.h;

    struct stat st = {0};

    int res = chdir(getenv("HOME"));
    if (res == 0 && stat(_desktopSdl2SettingsDir.c_str(), &st) == -1) {
        res = mkdir(_desktopSdl2SettingsDir.c_str(), 0777);
    }
    
    string cartdatadir = _desktopSdl2SettingsPrefix + "cdata";
    if (res == 0 && stat(cartdatadir.c_str(), &st) == -1) {
        res = mkdir(cartdatadir.c_str(), 0777);
    }
	
	#if KB_ENABLED
	SDL_StartTextInput();
	#endif 

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
    uint8_t kUp = 0;
    stretchKeyPressed = false;

    currKBDown = false;
	currKBKey = "";

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
			
			#if KB_ENABLED
			case SDL_TEXTINPUT:
				//Logger_Write( charset::upper_to_emoji(event.text.text).c_str() );
				//Logger_Write("\n");
				currKBKey = charset::upper_to_emoji(event.text.text);
				currKBDown = true;
				
				break;
			#endif
			
			
            case SDL_KEYDOWN:
			
				#if KB_ENABLED
				switch (event.key.keysym.scancode)
				{
					case SDL_SCANCODE_BACKSPACE: currKBDown = true; currKBKey = "\b"; break;
					case SDL_SCANCODE_RETURN: currKBDown = true; currKBKey = "\r"; break;
					case SDL_SCANCODE_ESCAPE: currKBDown = true; currKBKey = "\27"; break;
					case SDL_SCANCODE_TAB: currKBDown = true; currKBKey = "\t"; break;
					default : break;
				}
				#endif
				
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:case SDLK_RETURN:case SDLK_RETURN2:
                                     currKDown |= P8_KEY_PAUSE; break;
                    case SDLK_LEFT:  currKDown |= P8_KEY_LEFT; break;
                    case SDLK_RIGHT: currKDown |= P8_KEY_RIGHT; break;
                    case SDLK_UP:    currKDown |= P8_KEY_UP; break;
                    case SDLK_DOWN:  currKDown |= P8_KEY_DOWN; break;
                    case SDLK_z:     currKDown |= P8_KEY_X; break;
                    case SDLK_x:     currKDown |= P8_KEY_O; break;
                    case SDLK_c:     currKDown |= P8_KEY_X; break;
                    case SDLK_r:     stretchKeyPressed = true; break;
                    case SDLK_F2:    currKDown |= P8_KEY_7; break;

                    //case SDLK_F2:    currKBKey = "F2"; currKBDown = true; break;
                    //case SDLK_F4:    currKBKey = "F4"; currKBDown = true; break;
                }
                break;
            
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:case SDLK_RETURN:case SDLK_RETURN2:
                                     kUp |= P8_KEY_PAUSE; break;
                    case SDLK_LEFT:  kUp |= P8_KEY_LEFT; break;
                    case SDLK_RIGHT: kUp |= P8_KEY_RIGHT; break;
                    case SDLK_UP:    kUp |= P8_KEY_UP; break;
                    case SDLK_DOWN:  kUp |= P8_KEY_DOWN; break;
                    case SDLK_z:     kUp |= P8_KEY_X; break;
                    case SDLK_x:     kUp |= P8_KEY_O; break;
                    case SDLK_c:     kUp |= P8_KEY_X; break;
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

    currKHeld |= currKDown;
    currKHeld ^= kUp;
    
    return InputState_t {
        currKDown,
        currKHeld,
        (int16_t)mouseX,
        (int16_t)mouseY,
        picoMouseState,
		currKBDown,
		currKBKey
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


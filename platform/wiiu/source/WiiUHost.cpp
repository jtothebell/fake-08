/*
Wii U port todo:
1) sound messed up (turned off)
2) Fix buttons being very confusing
*/
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "../../../source/host.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"
#include "../../../source/logger.h"
#include "../../../source/filehelpers.h"

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#define SCREEN_SIZE_X 640
#define SCREEN_SIZE_Y 640

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720

#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define LSTICK    4
#define RSTICK    5
#define JOY_L     6
#define JOY_R     7
#define JOY_ZL    8
#define JOY_ZR    9
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2

int screenWidth = SCREEN_SIZE_X;
int screenHeight = SCREEN_SIZE_Y;

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;


uint32_t last_time;
uint32_t now_time;
uint32_t frame_time;
uint32_t targetFrameTimeMs;


Color* _paletteColors;

Audio* _audio;

SDL_Window* window;
SDL_Event event;
SDL_Renderer *renderer;
SDL_Texture *texture = NULL;
SDL_Rect DestR;
SDL_Rect SrcR;
double textureAngle;
SDL_RendererFlip flip;
SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;
void *pixels;
uint8_t *base;
int pitch;

SDL_Point touchLocation = { 128 / 2, 128 / 2 };



void postFlipFunction(){
    //flush switch frame buffers
    // We're done rendering, so we end the frame here.
    SDL_UnlockTexture(texture);
    SDL_RenderCopyEx(renderer, texture, &SrcR, &DestR, textureAngle, NULL, flip);

    SDL_RenderPresent(renderer);
}


bool audioInitialized = false;

void audioCleanup(){
    audioInitialized = false;

    SDL_CloseAudioDevice(dev);
}

void FillAudioDeviceBuffer(void* UserData, Uint8* DeviceBuffer, int Length)
{
    _audio->FillAudioBuffer(DeviceBuffer, 0, Length / 4);
}

void audioSetup(){
    //modifed from SDL docs: https://wiki.libsdl.org/SDL_OpenAudioDevice

    //Audio plays but is wrong. maybe a problem with sample rate or endian-ness? haven't investigated thoroughly
    SDL_memset(&want, 0, sizeof(want)); // or SDL_zero(want)
    want.freq = SAMPLERATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 4096;
    want.callback = FillAudioDeviceBuffer;
    

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (dev == 0) {
        Logger_Write("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) {
            Logger_Write("We didn't get requested audio format.");
        }
        SDL_PauseAudioDevice(dev, 0); // start audio playing.

        audioInitialized = true;
    }
}


void _changeStretch(StretchOption newStretch){
    if (newStretch == PixelPerfect) {
        screenWidth = PicoScreenWidth;
        screenHeight = PicoScreenHeight;
    }
    else if (newStretch == StretchToFit) {
        screenWidth = WIN_HEIGHT;
        screenHeight = WIN_HEIGHT;
    }
    else if (newStretch == StretchToFill) {
        screenWidth = WIN_WIDTH;
        screenHeight = WIN_HEIGHT; 
    }
    else if (newStretch == PixelPerfectStretch) {
        screenWidth = SCREEN_SIZE_X;
        screenHeight = SCREEN_SIZE_Y; 
    }
    else if (newStretch == FourByThreeVertPerfect) {
        screenWidth = SCREEN_SIZE_Y * 4 / 3;
        screenHeight = SCREEN_SIZE_Y; 
    }
    else if (newStretch == FourByThreeStretch) {
        screenWidth = WIN_HEIGHT * 4 / 3;
        screenHeight = WIN_HEIGHT; 
    }

    DestR.x = WIN_WIDTH / 2 - screenWidth / 2;
    DestR.y = WIN_HEIGHT / 2 - screenHeight / 2;
    DestR.w = screenWidth;
    DestR.h = screenHeight;

    SrcR.x = 0;
    SrcR.y = 0;
    SrcR.w = PicoScreenWidth;
    SrcR.h = PicoScreenHeight;

    textureAngle = 0;
    flip = SDL_FLIP_NONE;
}

Host::Host() {
    struct stat st = {0};

    int res = chdir("fs:/vol/external01");
    if (res == 0 && stat("wiiu", &st) == -1) {
        res = mkdir("wiiu", 0777);
    }
    if (res == 0 && stat("wiiu/apps", &st) == -1) {
        res = mkdir("wiiu/apps", 0777);
    }
    if (res == 0 && stat("wiiu/apps/fake08", &st) == -1) {
        res = mkdir("wiiu/apps/fake08", 0777);
    }
    if (res == 0 && stat("wiiu/apps/fake08/cdata", &st) == -1) {
        res = mkdir("wiiu/apps/fake08/cdata", 0777);
    }

    _logFilePrefix = "fs:/vol/external01/wiiu/apps/fake08/";
    _cartDirectory = "fs:/vol/external01/p8carts";
 }

  void Host::setPlatformParams(
        int windowWidth,
        int windowHeight,
        uint32_t sdlWindowFlags,
        uint32_t sdlRendererFlags,
        uint32_t sdlPixelFormat,
        std::string logFilePrefix,
        std::string customBiosLua,
        std::string cartDirectory) {}


void Host::oneTimeSetup(Color* paletteColors, Audio* audio){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL could not initialize\n");
        quit = 1;
        return;
    }

	window = SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!window) 
    { 
        quit = 1;
        return; 
    }
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) 
    { 
        quit = 1;
        return;
    }

	texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, PicoScreenWidth, PicoScreenHeight);
	if (texture == NULL) 
    {
		quit = 1;
        return;
	}

    atexit(SDL_Quit);

    DestR.x = WIN_WIDTH / 2 - SCREEN_SIZE_X / 2;
    DestR.y = WIN_HEIGHT / 2 - SCREEN_SIZE_Y / 2;
    DestR.w = SCREEN_SIZE_X;
    DestR.h = SCREEN_SIZE_Y;

    _audio = audio;
    audioSetup();

    for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (SDL_JoystickOpen(i) == NULL) {
			printf("Failed to open joystick %d!\n", i);
			quit = 1;
		}
    }

    stretch = PixelPerfectStretch;

    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    currKDown = 0;
    currKHeld = 0;

    _paletteColors = paletteColors;

    loadSettingsIni();

    _changeStretch(stretch);

    scaleX = screenWidth / (float)PicoScreenWidth;
    scaleY = screenHeight / (float)PicoScreenHeight;
    mouseOffsetX = DestR.x;
    mouseOffsetY = DestR.y;
}

void Host::oneTimeCleanup(){
    saveSettingsIni();
    
    audioCleanup();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Host::setTargetFps(int targetFps){
    targetFrameTimeMs = 1000 / targetFps;
}

void Host::changeStretch(){
    if (stretchKeyPressed) {
        if (stretch == PixelPerfectStretch) {
            stretch = PixelPerfect;
        }
        else if (stretch == PixelPerfect) {
            stretch = StretchToFit;
        }
        else if (stretch == StretchToFit) {
            stretch = StretchToFill;
        }
        else if (stretch == StretchToFill) {
            stretch = FourByThreeVertPerfect;
        }
        else if (stretch == FourByThreeVertPerfect) {
            stretch = FourByThreeStretch;
        }
        else if (stretch == FourByThreeStretch) {
            stretch = PixelPerfectStretch;
        }

        _changeStretch(stretch);

        scaleX = screenWidth / (float)PicoScreenWidth;
        scaleY = screenHeight / (float)PicoScreenHeight;
        mouseOffsetX = DestR.x;
        mouseOffsetY = DestR.y;
    }
}

InputState_t Host::scanInput(){ 
    currKDown = 0;
    uint8_t kUp = 0;
    stretchKeyPressed = false;

    uint8_t mouseBtnState = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYBUTTONDOWN :
                switch (event.jbutton.button)
                {
                    case JOY_PLUS:  currKDown |= P8_KEY_PAUSE; break;
                    case JOY_LEFT:  currKDown |= P8_KEY_LEFT; break;
                    case JOY_RIGHT: currKDown |= P8_KEY_RIGHT; break;
                    case JOY_UP:    currKDown |= P8_KEY_UP; break;
                    case JOY_DOWN:  currKDown |= P8_KEY_DOWN; break;
                    case JOY_A:     currKDown |= P8_KEY_X; break;
                    case JOY_B:     currKDown |= P8_KEY_O; break;

                    case JOY_L: lDown = true; break;
                    case JOY_R: rDown = true; break;
                    case JOY_MINUS: stretchKeyPressed = true; break;
                }
                break;

            case SDL_JOYBUTTONUP :
                switch (event.jbutton.button)
                {
                    case JOY_PLUS:  kUp |= P8_KEY_PAUSE; break;
                    case JOY_LEFT:  kUp |= P8_KEY_LEFT; break;
                    case JOY_RIGHT: kUp |= P8_KEY_RIGHT; break;
                    case JOY_UP:    kUp |= P8_KEY_UP; break;
                    case JOY_DOWN:  kUp |= P8_KEY_DOWN; break;
                    case JOY_A:     kUp |= P8_KEY_X; break;
                    case JOY_B:     kUp |= P8_KEY_O; break;

                    case JOY_L: lDown = false; break;
                    case JOY_R: rDown = false; break;
                }
               break;
            
            case SDL_FINGERDOWN:
                //touchId 0 is front, 1 is back. ignore back touches
                if (event.tfinger.touchId == 0) {
                    touchLocation.x = ((event.tfinger.x * WIN_WIDTH) - mouseOffsetX) / scaleX;
                    touchLocation.y = ((event.tfinger.y * WIN_HEIGHT) - mouseOffsetY) / scaleY;
                    mouseBtnState = 1;
                }
                break;
            
            case SDL_FINGERMOTION:
                //touchId 0 is front, 1 is back. ignore back touches
                if (event.tfinger.touchId == 0) {
                    touchLocation.x = ((event.tfinger.x * WIN_WIDTH) - mouseOffsetX) / scaleX;
                    touchLocation.y = ((event.tfinger.y * WIN_HEIGHT) - mouseOffsetY) / scaleY;
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

    return InputState_t {
        currKDown,
        currKHeld,
        (int16_t)touchLocation.x,
        (int16_t)touchLocation.y,
        mouseBtnState 
    };
    
}

bool Host::shouldQuit() {
    return quit > 0;
}

void Host::waitForTargetFps(){
    now_time = SDL_GetTicks();
    frame_time = now_time - last_time;
	last_time = now_time;


	//sleep for remainder of time
	if (frame_time < targetFrameTimeMs) {
		uint32_t msToSleep = targetFrameTimeMs - frame_time;
        
        SDL_Delay(msToSleep);

		last_time += msToSleep;
	}
}


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t drawMode){
    //clear screen to all black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    for (int y = 0; y < PicoScreenHeight; y ++){
        for (int x = 0; x < PicoScreenWidth; x ++){
            uint8_t c = getPixelNibble(x, y, picoFb);
            Color col = _paletteColors[screenPaletteMap[c]];

            base = ((Uint8 *)pixels) + (4 * ( y * PicoScreenHeight + x));
            base[0] = col.Red;
            base[1] = col.Green;
            base[2] = col.Blue;
            base[3] = col.Alpha;
        }
    }


    SrcR.x = 0;
    SrcR.y = 0;

    switch(drawMode){
        case 1:
            SrcR.w = 64;
            SrcR.h = PicoScreenHeight;
            textureAngle = 0;
            flip = SDL_FLIP_NONE;
            break;
        case 2:
            SrcR.w = PicoScreenWidth;
            SrcR.h = 64;
            textureAngle = 0;
            flip = SDL_FLIP_NONE;
            break;
        case 3:
            SrcR.w = 64;
            SrcR.h = 64;
            textureAngle = 0;
            flip = SDL_FLIP_NONE;
            break;
        //todo: mirroring
        //case 4,6,7
        case 129:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 0;
            flip = SDL_FLIP_HORIZONTAL;
            break;
        case 130:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 0;
            flip = SDL_FLIP_VERTICAL;
            break;
        case 131:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 0;
            flip = (SDL_RendererFlip)(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
            break;
        case 133:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 90;
            flip = SDL_FLIP_NONE;
            break;
        case 134:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 180;
            flip = SDL_FLIP_NONE;
            break;
        case 135:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 270;
            flip = SDL_FLIP_NONE;
            break;
        default:
            SrcR.w = PicoScreenWidth;
            SrcR.h = PicoScreenHeight;
            textureAngle = 0;
            flip = SDL_FLIP_NONE;
            break;
    }
    

    postFlipFunction();
}

bool Host::shouldFillAudioBuff(){
    return false;
}

void* Host::getAudioBufferPointer(){
    return nullptr;
}

size_t Host::getAudioBufferSize(){
    return 0;
}

void Host::playFilledAudioBuffer(){
}

bool Host::shouldRunMainLoop(){
    return !quit;
}

vector<string> Host::listcarts(){
    vector<string> carts;

    
    std::string cartDir = "p8carts";
    std::string container = "fs:/vol/external01/";
    std::string fullCartDir = container + cartDir;

    chdir(container.c_str());
    DIR* dir = opendir(cartDir.c_str());
    struct dirent *ent;
    if (dir != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name)){
                carts.push_back(_cartDirectory + "/" + ent->d_name);
            }
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }

    
    return carts;
}

const char* Host::logFilePrefix() {
    return _logFilePrefix.c_str();
}

std::string Host::customBiosLua() {
    return "cartpath = \"sd:/p8carts/\"\n"
        "pausebtn = \"+\"";
}

std::string Host::getCartDirectory() {
    return _cartDirectory;
}


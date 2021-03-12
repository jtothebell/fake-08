
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

#include <psp2/kernel/processmgr.h>
#include <psp2/io/fcntl.h>

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#define SCREEN_SIZE_X 512
#define SCREEN_SIZE_Y 512

#define WIN_WIDTH 960
#define WIN_HEIGHT 544


#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2

#define R_SUCCEEDED(res)   ((res)>=0)
#define R_FAILED(res)      ((res)<0)

//these controller button defines from: https://github.com/RossMeikleham/Vita_SDL2_Examples
//MIT license
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

//Vita filesystem helpers: https://github.com/joel16/VITAlbum
//MIT license
#define SDLK_VITA_SELECT 10
#define SDLK_VITA_START 11

int screenWidth = SCREEN_SIZE_X;
int screenHeight = SCREEN_SIZE_Y;

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;


StretchOption stretch = PixelPerfectStretch;
uint32_t last_time;
uint32_t now_time;
uint32_t frame_time;
uint32_t targetFrameTimeMs;

uint8_t currKDown;
uint8_t currKHeld;
bool lDown = false;
bool rDown = false;
bool stretchKeyPressed = false;

Color* _paletteColors;

Audio* _audio;

SDL_Window* window;
SDL_Event event;
SDL_Renderer *renderer;
SDL_Texture *texture = NULL;
SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;
int quit = 0;
void *pixels;
uint8_t *base;
int pitch;

SDL_Rect DestR;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;
int jxDir = 0;
int jyDir = 0;

void postFlipFunction(){
    // We're done rendering, so we end the frame here.
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, &DestR);

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

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = SAMPLERATE;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = 4096;
    want.callback = FillAudioDeviceBuffer;
    

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (dev == 0) {
        Logger_Write("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) { /* we let this one thing change. */
            Logger_Write("We didn't get requested audio format.");
        }
        SDL_PauseAudioDevice(dev, 0); /* start audio playing. */

        audioInitialized = true;
    }
}

Host::Host() { }


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

    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    _paletteColors = paletteColors;
}

void Host::oneTimeCleanup(){
    audioCleanup();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    sceKernelExitProcess(0);
}

void Host::setTargetFps(int targetFps){
    targetFrameTimeMs = 1000 / targetFps;
}

void Host::changeStretch(){
    if (stretchKeyPressed) {
        if (stretch == PixelPerfectStretch) {
            stretch = PixelPerfect;
            screenWidth = PicoScreenWidth;
            screenHeight = PicoScreenHeight;
        }
        else if (stretch == PixelPerfect) {
            stretch = StretchToFit;
            screenWidth = WIN_HEIGHT;
            screenHeight = WIN_HEIGHT;
        }
        else if (stretch == StretchToFit) {
            stretch = StretchToFill;
            screenWidth = WIN_WIDTH;
            screenHeight = WIN_HEIGHT; 
        }
        else if (stretch == StretchToFill) {
            stretch = PixelPerfectStretch;
            screenWidth = SCREEN_SIZE_X;
            screenHeight = SCREEN_SIZE_Y; 
        }

        DestR.x = WIN_WIDTH / 2 - screenWidth / 2;
        DestR.y = WIN_HEIGHT / 2 - screenHeight / 2;
        DestR.w = screenWidth;
        DestR.h = screenHeight;
    }
}

InputState_t Host::scanInput(){ 
    currKDown = 0;
    uint8_t kUp = 0;
    int prevJxDir = jxDir;
    int prevJyDir = jyDir;
    stretchKeyPressed = false;

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
                    case SDLK_VITA_RTRIGGER: rDown = true; stretchKeyPressed = true; break;
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
        currKHeld
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


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap){
    //clear screen to all black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    for (int y = 0; y < PicoScreenHeight; y ++){
        for (int x = 0; x < PicoScreenWidth; x ++){
            uint8_t c = getPixelNibble(x, y, picoFb);
            Color col = _paletteColors[screenPaletteMap[c]];

            base = ((Uint8 *)pixels) + (4 * ( y * PicoScreenHeight + x));
            base[0] = col.Alpha;
            base[1] = col.Blue;
            base[2] = col.Green;
            base[3] = col.Red;
        }
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
    std::string container = "ux0:/";
    std::string fullCartDir = container + cartDir;

    SceUID dir = 0;
    int ret = 0;

    if (R_SUCCEEDED(dir = sceIoDopen(fullCartDir.c_str()))) {
        
        do {
            SceIoDirent dirent;

            if (R_FAILED(ret = sceIoDread(dir, &dirent))) {
                Logger_Write("Error: sceIoDread(%s) failed: 0x%lx\n", fullCartDir.c_str(), ret);
                continue;
            }
            
            if (SCE_S_ISDIR(dirent.d_stat.st_mode)) {
                continue;
            }

            carts.push_back(fullCartDir + "/" + dirent.d_name);
        } while (ret > 0);

        sceIoDclose(dir);
    }

    return carts;
}

const char* Host::logFilePrefix() {
    return "ux0:/";
}

std::string Host::customBiosLua() {
    return "cartpath = \"ux0:/p8carts/\"\n"
        "selectbtn = \"x\"\n"
        "pausebtn = \"start\"";
}


/*
Wii u port todo:
(X) exit gracefully
(X) fix input- ish - still no start
(X) load files from sd card
4) sound
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

// sdl
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

#define SCREEN_SIZE_X 640
#define SCREEN_SIZE_Y 640

#define WIN_WIDTH 1280
#define WIN_HEIGHT 720


#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2

const int __screenWidth = SCREEN_SIZE_X;
const int __screenHeight = SCREEN_SIZE_Y;

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;


StretchOption stretch;
uint32_t last_time;
uint32_t now_time;
uint32_t frame_time;
uint32_t targetFrameTimeMs;

uint8_t currKDown;
uint8_t currKHeld;

Color* _paletteColors;

SDL_Window* window;
SDL_Event event;
SDL_Renderer *renderer;
SDL_Texture *texture = NULL;
int quit = 0;
void *pixels;
uint8_t *base;
int pitch;

SDL_Rect DestR;

void postFlipFunction(){
    //flush switch frame buffers
    // We're done rendering, so we end the frame here.
    

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, &DestR);

    SDL_RenderPresent(renderer);
}


void init_fill_buffer(void *audioBuffer,size_t offset, size_t size) {

	uint32_t *dest = (uint32_t*)audioBuffer;

	for (size_t i=0; i<size; i++) {
		dest[i] = 0;
	}

	//DSP_FlushDataCache(audioBuffer,size);

}

bool audioInitialized = false;
uint32_t *audioBuffer;
uint32_t audioBufferSize;
//ndspWaveBuf waveBuf[2];
bool fillBlock = false;
uint32_t currPos;


void audioCleanup(){
    audioInitialized = false;

    //ndspExit();

    if(audioBuffer != nullptr) {
        //linearFree(audioBuffer);
        audioBuffer = nullptr;
    }
}

void audioSetup(){

}

Host::Host() { }


void Host::oneTimeSetup(Color* paletteColors){

    // ----- Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL could not initialize\n");
        quit = 1;
        return;
    }

    //Setup window
	window = SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!window) 
    { 
        quit = 1;
        return; 
    }
	
	//Setup renderer
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
}

void Host::setTargetFps(int targetFps){
    targetFrameTimeMs = 1000 / targetFps;
}

void Host::changeStretch(){
}

InputState_t Host::scanInput(){ 
    currKDown = 0;
    uint8_t kUp = 0;
    //Logger::Write("Scan input\n");


//check input here (call open joystick):
//https://github.com/ulquiorra-dev/Simple_SDL_Snake_WiiU_Port/blob/master/source/main.c
//tetris calls open joystick:
//https://github.com/ulquiorra-dev/SDL_TETRIS_WiiU_Port/blob/master/src/main.c

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_JOYBUTTONDOWN :
                switch (event.jbutton.button)
                {
                    case SDL_CONTROLLER_BUTTON_BACK:
                    case SDL_CONTROLLER_BUTTON_GUIDE:
                    case SDL_CONTROLLER_BUTTON_X:
                    case SDL_CONTROLLER_BUTTON_START:currKDown |= P8_KEY_PAUSE; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  currKDown |= P8_KEY_LEFT; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: currKDown |= P8_KEY_RIGHT; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:    currKDown |= P8_KEY_UP; break;
                    case SDL_CONTROLLER_BUTTON_MAX:  currKDown |= P8_KEY_DOWN; break;
                    case SDL_CONTROLLER_BUTTON_A:     currKDown |= P8_KEY_X; break;
                    case SDL_CONTROLLER_BUTTON_B:     currKDown |= P8_KEY_O; break;
                    case SDL_CONTROLLER_BUTTON_Y: quit = 1; break;
                }
                break;

            case SDL_JOYBUTTONUP :
                switch (event.jbutton.button)
                {
                    case SDL_CONTROLLER_BUTTON_BACK:
                    case SDL_CONTROLLER_BUTTON_GUIDE:
                    case SDL_CONTROLLER_BUTTON_X:
                    case SDL_CONTROLLER_BUTTON_START:kUp |= P8_KEY_PAUSE; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  kUp |= P8_KEY_LEFT; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: kUp |= P8_KEY_RIGHT; break;
                    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:    kUp |= P8_KEY_UP; break;
                    case SDL_CONTROLLER_BUTTON_MAX:  kUp |= P8_KEY_DOWN; break;
                    case SDL_CONTROLLER_BUTTON_A:     kUp |= P8_KEY_X; break;
                    case SDL_CONTROLLER_BUTTON_B:     kUp |= P8_KEY_O; break;
                }
               break;

            case SDL_QUIT:
                quit = 1;
                break;
        }
    }

    currKHeld |= currKDown;
    currKHeld ^= kUp;

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
            base[0] = col.Red;
            base[1] = col.Green;
            base[2] = col.Blue;
            base[3] = col.Alpha;
        }
    }
    

    postFlipFunction();
}

bool Host::shouldFillAudioBuff(){
    //return waveBuf[fillBlock].status == NDSP_WBUF_DONE;
    return false;
}

void* Host::getAudioBufferPointer(){
    //return waveBuf[fillBlock].data_pcm16;
    return nullptr;
}

size_t Host::getAudioBufferSize(){
    //return waveBuf[fillBlock].nsamples;
    return 0;
}

void Host::playFilledAudioBuffer(){
    //DSP_FlushDataCache(waveBuf[fillBlock].data_pcm16, waveBuf[fillBlock].nsamples);

	//ndspChnWaveBufAdd(0, &waveBuf[fillBlock]);

	//fillBlock = !fillBlock;
}

bool Host::shouldRunMainLoop(){

    //check how tetris handles update loop - quit flag
    //https://github.com/ulquiorra-dev/SDL_TETRIS_WiiU_Port/blob/master/src/main.c
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
            carts.push_back(fullCartDir + "/" + ent->d_name);
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }

    
    return carts;
}

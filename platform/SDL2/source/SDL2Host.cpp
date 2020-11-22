
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "../../../source/host.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"

// sdl
#include <SDL2/SDL.h>

#define SCREEN_SIZE_X 512
#define SCREEN_SIZE_Y 512


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
SDL_bool done = SDL_FALSE;
void *pixels;
uint8_t *base;
int pitch;


void postFlipFunction(){
    //flush switch frame buffers
    // We're done rendering, so we end the frame here.
    

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

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

    audioSetup();

    // ----- Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "SDL could not initialize\n");
        return;
    }

    // ----- Create window
    SDL_CreateWindowAndRenderer(SCREEN_SIZE_X, SCREEN_SIZE_Y, 0, &window, &renderer);
    if (!window)
    {
        fprintf(stderr, "Error creating window.\n");
        return;
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, PicoScreenWidth, PicoScreenHeight);
    if (!texture)
    {
        fprintf(stderr, "Error creating texture.\n");
        return;
    }
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    _paletteColors = paletteColors;
}

void Host::oneTimeCleanup(){
    audioCleanup();

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
    currKHeld = 0;

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
                }
                break;
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
        }
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
        currKHeld
    };
}

bool Host::shouldQuit() {
    return done == SDL_TRUE;
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
            base[0] = col.Blue;
            base[1] = col.Green;
            base[2] = col.Red;
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

	fillBlock = !fillBlock;
}

bool Host::shouldRunMainLoop(){
    if (shouldQuit()){
        return false;
    }

    return true;
}

vector<string> Host::listcarts(){
    vector<string> carts;

    DIR *dir;
    struct dirent *ent;
    std::string home = getenv("HOME");
    std::string cartDir = "/p8carts";
    std::string fullCartDir = home + cartDir;
    if ((dir = opendir (fullCartDir.c_str())) != NULL) {
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

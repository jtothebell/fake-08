
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
#include "../../../source/logger.h"

// sdl
#include <SDL/SDL.h>

#define SCREEN_BPP 16

#define SAMPLERATE 22050
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
Audio* _audio;

SDL_Event event;
SDL_Surface *window;
SDL_Surface *texture;
SDL_bool done = SDL_FALSE;
SDL_AudioSpec want, have;
void *pixels;
uint16_t *base;
int pitch;

bool audioInitialized = false;

uint16_t _mapped16BitColors[144];


void postFlipFunction(){
    // We're done rendering, so we end the frame here.

    SDL_SoftStretch(texture, nullptr, window, nullptr);

    SDL_Flip(window);
}

void audioCleanup(){
    audioInitialized = false;

    SDL_CloseAudio();
}


void FillAudioDeviceBuffer(void* UserData, Uint8* DeviceBuffer, int Length)
{
    _audio->FillMonoAudioBuffer(DeviceBuffer, 0, Length / 2);
}

void audioSetup(){
    //modifed from SDL docs: https://wiki.libsdl.org/SDL_OpenAudioDevice

    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLERATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = FillAudioDeviceBuffer;
    

    int audioOpenRes = SDL_OpenAudio(&want, &have);
    if (audioOpenRes < 0) {
        Logger_Write("Failed to open audio: %s", SDL_GetError());
    } else {
        if (have.format != want.format) { 
            Logger_Write("We didn't get requested audio format.");
        }
        SDL_PauseAudio(0); 
        audioInitialized = true;
    }
}


Host::Host() {
    #ifdef _BITTBOY
    _cartDirectory = "/mnt/roms/PICO-8";
    #else
    std::string home = getenv("HOME");
    
    _cartDirectory = home + "/p8carts";
    #endif
 }

void Host::oneTimeSetup(Color* paletteColors, Audio* audio){
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL could not initialize\n");
        return;
    }

    SDL_WM_SetCaption("FAKE-08", NULL);
    SDL_ShowCursor(SDL_DISABLE);

    int flags = SDL_SWSURFACE;

    window = SDL_SetVideoMode(0, 0, SCREEN_BPP, flags);

    texture = SDL_CreateRGBSurface(flags, PicoScreenWidth, PicoScreenHeight, SCREEN_BPP, 0, 0, 0, 0);

    _audio = audio;
    audioSetup();
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    _paletteColors = paletteColors;

    SDL_PixelFormat *f = window->format;

    for(int i = 0; i < 144; i++){
        _mapped16BitColors[i] = SDL_MapRGB(f, _paletteColors[i].Red, _paletteColors[i].Green, _paletteColors[i].Blue);
    }
}

void Host::oneTimeCleanup(){
    audioCleanup();

    //SDL_DestroyRenderer(renderer);
    //SDL_DestroyWindow(window);

    SDL_FreeSurface(texture);
    SDL_FreeSurface(window);

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
                    case SDLK_RETURN:currKDown |= P8_KEY_PAUSE; break;
                    case SDLK_l:  currKDown |= P8_KEY_LEFT; break;
                    case SDLK_r: currKDown |= P8_KEY_RIGHT; break;
                    case SDLK_u:    currKDown |= P8_KEY_UP; break;
                    case SDLK_d:  currKDown |= P8_KEY_DOWN; break;
                    case SDLK_b: currKDown |= P8_KEY_X; break;
                    case SDLK_a:currKDown |= P8_KEY_O; break;
                    case SDLK_x:  currKDown |= P8_KEY_X; break;
                    case SDLK_y: currKDown |= P8_KEY_O; break;
                    case SDLK_q: case SDLK_HOME: done = SDL_TRUE; break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_q:
						done = SDL_TRUE;
                    break;
                    default: break;
                }
                break;
            break;
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
        }
    }

    const Uint8* keystate = SDL_GetKeyState(NULL);

    //continuous-response keys
    if(keystate[SDLK_l]){
        currKHeld |= P8_KEY_LEFT;
    }
    if(keystate[SDLK_r]){
        currKHeld |= P8_KEY_RIGHT;;
    }
    if(keystate[SDLK_u]){
        currKHeld |= P8_KEY_UP;
    }
    if(keystate[SDLK_d]){
        currKHeld |= P8_KEY_DOWN;
    }
    if(keystate[SDLK_b]){
        currKHeld |= P8_KEY_X;
    }
    if(keystate[SDLK_a]){
        currKHeld |= P8_KEY_O;
    }
    if(keystate[SDLK_x]){
        currKHeld |= P8_KEY_X;
    }
    if(keystate[SDLK_y]){
        currKHeld |= P8_KEY_O;
    }
    if(keystate[SDLK_s]){
        currKHeld |= P8_KEY_PAUSE;
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

/*
void set_pixel(SDL_Surface *surface, int x, int y, uint16_t pixel)
{
  uint16_t * const target_pixel = (uint16_t *) ((Uint8 *) surface->pixels
                                             + y * surface->pitch
                                             + x * surface->format->BytesPerPixel);
  *target_pixel = pixel;
}
*/

void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t drawMode){
    
    pixels = texture->pixels;

    for (int y = 0; y < (PicoScreenHeight); y ++){
        for (int x = 0; x < PicoScreenWidth; x ++){
            uint8_t c = getPixelNibble(x, y, picoFb);
            uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

            base = ((uint16_t *)pixels) + ( y * PicoScreenHeight + x);
            base[0] = col;
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
    if (shouldQuit()){
        return false;
    }

    return true;
}

vector<string> Host::listcarts(){
    vector<string> carts;

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (_cartDirectory.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            carts.push_back(_cartDirectory + "/" + ent->d_name);
        }
        closedir (dir);
    } else {
        /* could not open directory */
        perror ("");
    }

    
    return carts;
}

const char* Host::logFilePrefix() {
    return "";
}

std::string Host::customBiosLua() {
    return "cartpath = \"roms/pico-8/\"\n"
        "selectbtn = \"z\"\n"
        "pausebtn = \"esc\""
        "exitbtn = \"close window\""
        "sizebtn = \"\"";
}

std::string Host::getCartDirectory() {
    return _cartDirectory;
}

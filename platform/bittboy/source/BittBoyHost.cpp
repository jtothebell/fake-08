
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

#define SCREEN_SIZE_X 320
#define SCREEN_SIZE_Y 240

#define SCREEN_BPP 16


#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2

const int __screenWidth = SCREEN_SIZE_X;
const int __screenHeight = SCREEN_SIZE_Y;

int _windowWidth = 128;
int _windowHeight = 128;

int _screenWidth = 128;
int _screenHeight = 128;

int _maxNoStretchWidth = 128;
int _maxNoStretchHeight = 128;

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

SDL_Rect SrcR;
SDL_Rect DestR;

double textureAngle;
uint8_t flip; //0 none, 1 horizontal, 2 vertical - match SDL2's SDL_RendererFlip

bool audioInitialized = false;

uint16_t _mapped16BitColors[144];


void postFlipFunction(){
    // We're done rendering, so we end the frame here.

    SDL_SoftStretch(texture, &SrcR, window, &DestR);

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
    want.format = AUDIO_S16LSB;
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

void _changeStretch(StretchOption newStretch){
    int srcx = 0;
    int srcy = 0;
    if (newStretch == PixelPerfect) {
        _screenWidth = PicoScreenWidth;
        _screenHeight = PicoScreenHeight;
    }
    else if (newStretch == StretchToFill){
        _screenWidth = _windowWidth;
        _screenHeight = _windowHeight; 
    }
    else if (newStretch == StretchAndOverflow) {
        _screenWidth = PicoScreenWidth * 2;
        _screenHeight = (PicoScreenHeight - 8) * 2;
        srcy = 4;
    }
    

    DestR.x = _windowWidth / 2 - _screenWidth / 2;
    DestR.y = _windowHeight / 2 - _screenHeight / 2;
    DestR.w = _screenWidth;
    DestR.h = _screenHeight;

    SrcR.x = srcx;
    SrcR.y = srcy;
    SrcR.w = PicoScreenWidth - (srcx * 2);
    SrcR.h = PicoScreenHeight - (srcy * 2);

    textureAngle = 0;
    flip = 0;

    //clear the screen so nothing is left over behind current stretch
    SDL_FillRect(window, NULL, SDL_MapRGB(window->format, 0, 0, 0));
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

    window = SDL_SetVideoMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, SCREEN_BPP, flags);

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


    _windowWidth = SCREEN_SIZE_X;
    _windowHeight = SCREEN_SIZE_Y;

    //TODO: store in settings INI
    stretch = StretchToFill;

    _changeStretch(stretch);
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
    if (stretchKeyPressed) {
        StretchOption newStretch = stretch;

        if (stretch == StretchAndOverflow) {
            newStretch = PixelPerfect;
        }
        else if (stretch == PixelPerfect) {
            newStretch = StretchToFill;
        }
        else if (stretch == StretchToFill) {
            newStretch = StretchAndOverflow;
        }

        _changeStretch(newStretch);

        stretch = newStretch;
        scaleX = _screenWidth / (float)PicoScreenWidth;
        scaleY = _screenHeight / (float)PicoScreenHeight;
        mouseOffsetX = DestR.x;
        mouseOffsetY = DestR.y;
    }
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
                    case SDLK_RETURN:currKDown |= P8_KEY_PAUSE; break;
                    case SDLK_LEFT:  currKDown |= P8_KEY_LEFT; break;
                    case SDLK_RIGHT: currKDown |= P8_KEY_RIGHT; break;
                    case SDLK_UP:    currKDown |= P8_KEY_UP; break;
                    case SDLK_DOWN:  currKDown |= P8_KEY_DOWN; break;
                    case SDLK_SPACE: currKDown |= P8_KEY_X; break;
                    case SDLK_LSHIFT:currKDown |= P8_KEY_O; break;
                    case SDLK_LALT:  currKDown |= P8_KEY_X; break;
                    case SDLK_LCTRL: currKDown |= P8_KEY_O; break;
                    case SDLK_RCTRL: done = SDL_TRUE; break;
                    case SDLK_ESCAPE: stretchKeyPressed = true; break;
                    default: break;
                }
                break;
            case SDL_QUIT:
                done = SDL_TRUE;
                break;
        }
    }

    const Uint8* keystate = SDL_GetKeyState(NULL);

    //continuous-response keys
    if(keystate[SDLK_LEFT]){
        currKHeld |= P8_KEY_LEFT;
    }
    if(keystate[SDLK_RIGHT]){
        currKHeld |= P8_KEY_RIGHT;;
    }
    if(keystate[SDLK_UP]){
        currKHeld |= P8_KEY_UP;
    }
    if(keystate[SDLK_DOWN]){
        currKHeld |= P8_KEY_DOWN;
    }
    if(keystate[SDLK_SPACE]){
        currKHeld |= P8_KEY_X;
    }
    if(keystate[SDLK_LSHIFT]){
        currKHeld |= P8_KEY_O;
    }
    if(keystate[SDLK_LALT]){
        currKHeld |= P8_KEY_X;
    }
    if(keystate[SDLK_LCTRL]){
        currKHeld |= P8_KEY_O;
    }
    if(keystate[SDLK_RETURN]){
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

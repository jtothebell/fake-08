
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <fstream>
#include <iostream>
using namespace std;

#include "../../../source/host.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"
#include "../../../source/logger.h"
#include "../../../source/filehelpers.h"

// sdl
#include <SDL/SDL.h>

#define SCREEN_SIZE_X 640
#define SCREEN_SIZE_Y 480

#define SCREEN_BPP 16

#define SAMPLERATE 22050

const int __screenWidth = SCREEN_SIZE_X;
const int __screenHeight = SCREEN_SIZE_Y;

int _windowWidth = 128;
int _windowHeight = 128;

int _screenWidth = 128;
int _screenHeight = 128;

int _maxNoStretchWidth = 384;
int _maxNoStretchHeight = 384;

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;
const int pixelBlocksPerLine = PicoScreenWidth / 8;


StretchOption stretch;
uint32_t last_time;
uint32_t now_time;
uint32_t frame_time;
uint32_t targetFrameTimeMs;

uint8_t currKDown;
uint8_t currKHeld;

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

int textureAngle;
uint8_t flip; //0 none, 1 horizontal, 2 vertical - match SDL2's SDL_RendererFlip
int drawModeScaleX = 1;
int drawModeScaleY = 1;

bool audioInitialized = false;

uint16_t _mapped16BitColors[144];

/*
 * Gameblabla 
 * Well unfortunately we still have IPU bugs/issues due to how the IPU scaler works.
 * So for now, we have to use a simple software scaler. (A performance penalty may occur on the RG-300X/RG-350M)
*/
//#define OPENDINGUX_IPU 1


void postFlipFunction(){
    // We're done rendering, so we end the frame here.
#ifdef OPENDINGUX_IPU
    SDL_Flip(window);
#else
    SDL_SoftStretch(texture, &SrcR, window, &DestR);
    SDL_Flip(window);
#endif
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

void _setSourceRect(int xoffset, int yoffset) {
    SrcR.x = xoffset;
    SrcR.y = yoffset;
    SrcR.w = PicoScreenWidth / drawModeScaleX - (xoffset * 2);
    SrcR.h = PicoScreenHeight / drawModeScaleY - (yoffset * 2);
}

void _changeStretch(StretchOption newStretch){
    int xoffset = 0;
    int yoffset = 0;

    if (newStretch == PixelPerfect) {
        _screenWidth = PicoScreenWidth;
        _screenHeight = PicoScreenHeight;
    }
    else if (newStretch == StretchToFit) {
        _screenWidth = _windowHeight;
        _screenHeight = _windowHeight;
    }
    else if (newStretch == StretchToFill){
        _screenWidth = _windowWidth;
        _screenHeight = _windowHeight; 
    }
    else if (newStretch == PixelPerfectStretch) {
        _screenWidth = _maxNoStretchWidth;
        _screenHeight = _maxNoStretchHeight; 
    }
    else if (newStretch == FourByThreeVertPerfect) {
        _screenWidth = _maxNoStretchHeight * 4 / 3;
        _screenHeight = _maxNoStretchHeight; 
    }
    else if (newStretch == StretchAndOverflow) {
        yoffset = 4 / drawModeScaleY;
        _screenWidth = PicoScreenWidth * 4;
        _screenHeight = _windowHeight;
    }
    //default to StretchToFill)
    else {
        _screenWidth = _windowWidth;
        _screenHeight = _windowHeight; 
    }
    

    DestR.x = _windowWidth / 2 - _screenWidth / 2;
    DestR.y = _windowHeight / 2 - _screenHeight / 2;
    DestR.w = _screenWidth;
    DestR.h = _screenHeight;

    _setSourceRect(xoffset, yoffset);

    textureAngle = 0;
    flip = 0;

    //clear the screen so nothing is left over behind current stretch
    SDL_FillRect(window, NULL, SDL_MapRGB(window->format, 0, 0, 0));
}






Host::Host() {
    #ifdef _GCW0
    _cartDirectory = "/media/sdcard/roms/PICO8";
    _logFilePrefix = "/media/sdcard/roms/PICO8/";
    #else
    std::string home = getenv("HOME");
    
    _cartDirectory = home + "/p8carts";
    _logFilePrefix = home + "/fake08";
    #endif

    struct stat st = {0};
    int res = 0;

    string cartdatadir = _logFilePrefix + "cdata";
    if (stat(cartdatadir.c_str(), &st) == -1) {
        res = mkdir(cartdatadir.c_str(), 0777);
    }
 }

void Host::oneTimeSetup(Audio* audio){
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "SDL could not initialize\n");
        return;
    }

    SDL_WM_SetCaption("FAKE-08", NULL);
    SDL_ShowCursor(SDL_DISABLE);

    int flags = SDL_HWSURFACE;

	#ifdef OPENDINGUX_IPU
    window = SDL_SetVideoMode(128, 128, SCREEN_BPP, flags);
    #else
    //todo: 0, 0 video mode, then check later to get actual resolution. handle 320x240 and 640x40
    window = SDL_SetVideoMode(SCREEN_SIZE_X, SCREEN_SIZE_Y, SCREEN_BPP, flags);
    texture = SDL_CreateRGBSurface(flags, PicoScreenWidth, PicoScreenHeight, SCREEN_BPP, 0, 0, 0, 0);
    #endif

    _audio = audio;
    audioSetup();
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    SDL_PixelFormat *f = window->format;

    for(int i = 0; i < 144; i++){
        _mapped16BitColors[i] = SDL_MapRGB(f, _paletteColors[i].Red, _paletteColors[i].Green, _paletteColors[i].Blue);
    }

    const SDL_VideoInfo* info = SDL_GetVideoInfo();
    _windowWidth = info->current_w;
    _windowHeight = info->current_h;

    if (_windowWidth < _maxNoStretchWidth || _windowHeight < _maxNoStretchHeight){
        _maxNoStretchWidth = _maxNoStretchHeight = 128;
    }

    //TODO: store in settings INI
    stretch = StretchToFill;
    loadSettingsIni();

    _changeStretch(stretch);
}

void Host::oneTimeCleanup(){
    audioCleanup();

    //saving seems to increase the number of hard locks
    saveSettingsIni();

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

        if (stretch == PixelPerfectStretch) {
            newStretch = StretchToFit;
        }
        else if (stretch == StretchToFit) {
            newStretch = StretchToFill;
        }
        else if (stretch == StretchToFill) {
            newStretch = StretchAndOverflow;
        }
        else if (stretch == StretchAndOverflow) {
            newStretch = PixelPerfectStretch;
        }
        else {
            newStretch = PixelPerfectStretch;
        }

        _changeStretch(newStretch);

        stretch = newStretch;
        scaleX = _screenWidth / (float)PicoScreenWidth;
        scaleY = _screenHeight / (float)PicoScreenHeight;
        mouseOffsetX = DestR.x;
        mouseOffsetY = DestR.y;
    }
}

void Host::forceStretch(StretchOption newStretch) {
	_changeStretch(newStretch);
	stretch = newStretch;
	scaleX = _screenWidth / (float)PicoScreenWidth;
	scaleY = _screenHeight / (float)PicoScreenHeight;
	mouseOffsetX = DestR.x;
	mouseOffsetY = DestR.y;
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
                    case SDLK_HOME: done = SDL_TRUE; break;
                    case SDLK_ESCAPE: stretchKeyPressed = true; break;
                    default: break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym)
                {
                    case SDLK_HOME:
                    #ifdef GKD
                    case SDLK_TAB:
                    #endif
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
    #ifdef OPENDINGUX_IPU
    pixels = window->pixels;
    #else
	drawModeScaleX = 1;
    drawModeScaleY = 1;
    switch(drawMode){
        case 1:
            drawModeScaleX = 2;
            textureAngle = 0;
            flip = 0;
            break;
        case 2:
            drawModeScaleY = 2;
            textureAngle = 0;
            flip = 0;
            break;
        case 3:
            drawModeScaleX = 2;
            drawModeScaleY = 2;
            textureAngle = 0;
            flip = 0;
            break;
        //todo: mirroring
        //case 4,6,7
        case 129:
            textureAngle = 0;
            flip = 1;
            break;
        case 130:
            textureAngle = 0;
            flip = 2;
            break;
        case 131:
            textureAngle = 0;
            flip = 3;
            break;
        case 133:
            textureAngle = 90;
            flip = 0;
            break;
        case 134:
            textureAngle = 180;
            flip = 0;
            break;
        case 135:
            textureAngle = 270;
            flip = 0;
            break;
        default:
            
            textureAngle = 0;
            flip = 0;
            break;
    }
    int yoffset = stretch == StretchAndOverflow ? 4 / drawModeScaleX : 0;

    _setSourceRect(0, yoffset);

    pixels = texture->pixels;

    //horizontal flip
    if (textureAngle == 0 && flip == 1) {
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < PicoScreenWidth; x ++){
                uint8_t c = getPixelNibble(x, y, picoFb);
                uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

                base = ((uint16_t *)pixels) + ( y * PicoScreenHeight + (127 - x));
                base[0] = col;
            }
        }
    }
    //vertical flip
    else if (textureAngle == 0 && flip == 2) {
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < PicoScreenWidth; x ++){
                uint8_t c = getPixelNibble(x, y, picoFb);
                uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

                base = ((uint16_t *)pixels) + ((127 - y) * PicoScreenHeight + x);
                base[0] = col;
            }
        }
    }
    //horizontal and vertical flip
    else if (textureAngle == 0 && flip == 3) {
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < PicoScreenWidth; x ++){
                uint8_t c = getPixelNibble(x, y, picoFb);
                uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

                base = ((uint16_t *)pixels) + ((127 - y) * PicoScreenHeight + (127 - x));
                base[0] = col;
            }
        }
    }
    //rotated 90 degrees
    else if (textureAngle == 90 && flip == 0) { 
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < PicoScreenWidth; x ++){
                uint8_t c = getPixelNibble(x, y, picoFb);
                uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

                base = ((uint16_t *)pixels) + (x * PicoScreenHeight + (127 - y));
                base[0] = col;
            }
        }
    }
    //rotated 180 degrees
    else if (textureAngle == 180 && flip == 0) { 
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < PicoScreenWidth; x ++){
                uint8_t c = getPixelNibble(x, y, picoFb);
                uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

                base = ((uint16_t *)pixels) + ((127 - y) * PicoScreenHeight + (127 - x));
                base[0] = col;
            }
        }
    }
    //rotated 270 degrees
    else if (textureAngle == 270 && flip == 0) { 
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < PicoScreenWidth; x ++){
                uint8_t c = getPixelNibble(x, y, picoFb);
                uint16_t col = _mapped16BitColors[screenPaletteMap[c]];

                base = ((uint16_t *)pixels) + ((127 - x) * PicoScreenHeight + y);
                base[0] = col;
            }
        }
    }
    else { //default
        for (int y = 0; y < PicoScreenHeight; y ++){
            for (int x = 0; x < pixelBlocksPerLine; x ++){
                int32_t eightPix = ((int32_t*)picoFb)[y * pixelBlocksPerLine + x];

                int h = (eightPix >> 28) & 0x0f;
                int g = (eightPix >> 24) & 0x0f;
                int f = (eightPix >> 20) & 0x0f;
                int e = (eightPix >> 16) & 0x0f;
                int d = (eightPix >> 12) & 0x0f;
                int c = (eightPix >>  8) & 0x0f;
                int b = (eightPix >>  4) & 0x0f;
                int a = (eightPix)       & 0x0f;

                int32_t cola = _mapped16BitColors[screenPaletteMap[a]];
                int32_t colb = _mapped16BitColors[screenPaletteMap[b]];
                int32_t colc = _mapped16BitColors[screenPaletteMap[c]];
                int32_t cold = _mapped16BitColors[screenPaletteMap[d]];
                int32_t cole = _mapped16BitColors[screenPaletteMap[e]];
                int32_t colf = _mapped16BitColors[screenPaletteMap[f]];
                int32_t colg = _mapped16BitColors[screenPaletteMap[g]];
                int32_t colh = _mapped16BitColors[screenPaletteMap[h]];

                
                base = ((uint16_t *)pixels + (y * PicoScreenHeight + x * 8));
                base[0] = cola;
                base[1] = colb;
                base[2] = colc;
                base[3] = cold;
                base[4] = cole;
                base[5] = colf;
                base[6] = colg;
                base[7] = colh;
                

                //----OR something like this for further optimization?
                //not exactly this. its broken
                /*
                int32_t* base32 = ((int32_t *)pixels + (y * PicoScreenHeight + x * 8));
                base32[0] = cola << 16 & colb;
                base32[1] = colc << 16 & cold;
                base32[2] = cole << 16 & colf;
                base32[3] = colg << 16 & colh;
                */
                
                
            }
        }
    }
    #endif

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
    return "";
}

std::string Host::customBiosLua() {
    return "cartpath = \"roms/PICO8/\"\n"
        "selectbtn = \"a\"\n"
        "pausebtn = \"start\""
        "exitbtn = \"power\""
        "sizebtn = \"select\"";
}

std::string Host::getCartDirectory() {
    return _cartDirectory;
}

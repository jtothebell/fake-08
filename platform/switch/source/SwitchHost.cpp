#include <switch.h>
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

#define FB_WIDTH  1280
#define FB_HEIGHT 720

#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2

const int __screenWidth = FB_WIDTH;
const int __screenHeight = FB_HEIGHT;

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;


StretchOption stretch = PixelPerfectStretch;
u64 last_time;
u64 now_time;
u64 frame_time;
double targetFrameTimeMs;

u32 currKDown;
u32 currKHeld;

Framebuffer fb;

Color* _paletteColors;
Audio* _audio;

uint8_t ConvertInputToP8(u32 input){
	uint8_t result = 0;
	if (input & KEY_LEFT){
		result |= P8_KEY_LEFT;
	}

	if (input & KEY_RIGHT){
		result |= P8_KEY_RIGHT;
	}

	if (input & KEY_UP){
		result |= P8_KEY_UP;
	}

	if (input & KEY_DOWN){
		result |= P8_KEY_DOWN;
	}

	if (input & KEY_B){
		result |= P8_KEY_O;
	}

	if (input & KEY_A){
		result |= P8_KEY_X;
	}

	if (input & KEY_PLUS){
		result |= P8_KEY_PAUSE;
	}

	if (input & KEY_MINUS){
		result |= P8_KEY_7;
	}

	return result;
}

void postFlipFunction(){
    //flush switch frame buffers
    // We're done rendering, so we end the frame here.
    framebufferEnd(&fb);
}


bool audioInitialized = false;


void audioCleanup(){
    audioInitialized = false;
}

void audioSetup(){
    
}



Host::Host() { }


void Host::oneTimeSetup(Color* paletteColors, Audio* audio){

    NWindow* win = nwindowGetDefault();

    framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2);
    framebufferMakeLinear(&fb);

    _audio = audio;
    audioSetup();
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    _paletteColors = paletteColors;
}

void Host::oneTimeCleanup(){
    audioCleanup();

	framebufferClose(&fb);
}

void Host::setTargetFps(int targetFps){
    targetFrameTimeMs = 1000.0 / (double)targetFps;
}

void Host::changeStretch(){
    if (currKDown & KEY_R) {
        if (stretch == PixelPerfect) {
            stretch = PixelPerfectStretch;
        }
        else if (stretch == PixelPerfectStretch) {
            stretch = PixelPerfect;
        }
    }
}

InputState_t Host::scanInput(){
    hidScanInput();

    currKDown = hidKeysDown(CONTROLLER_P1_AUTO);
    currKHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

    return InputState_t {
        ConvertInputToP8(currKDown),
        ConvertInputToP8(currKHeld)
    };
}


bool Host::shouldQuit() {
    bool lpressed = currKHeld & KEY_L;
	bool rpressed = currKDown & KEY_R;

	return lpressed && rpressed;
}

const double NANOSECONDS_TO_MILLISECONDS = 1.0 / 1000000.0;

void Host::waitForTargetFps(){
    now_time = armGetSystemTick();
    frame_time = now_time - last_time;
	last_time = now_time;

    u64 frameTimeNs = armTicksToNs(frame_time);
	double frameTimeMs = frameTimeNs * NANOSECONDS_TO_MILLISECONDS;

	//sleep for remainder of time
	if (frameTimeMs < targetFrameTimeMs) {
		double msToSleep = targetFrameTimeMs - frameTimeMs;
        u64 nsToSleep = msToSleep * 1000 * 1000;

		svcSleepThread(nsToSleep);

		last_time += armNsToTicks(nsToSleep);
	}
}


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap){
	u32 stride;
    u32* framebuf = (u32*) framebufferBegin(&fb, &stride);

	//clear frame buf
	memset(framebuf, 0, __screenHeight*__screenWidth*4);


    u32 renderWidth = PicoScreenWidth;
    u32 renderHeight = PicoScreenHeight;
    u32 ratio = 1;

    if (stretch == PixelPerfectStretch){
        ratio = FB_HEIGHT / PicoScreenHeight; //should be 5
        renderWidth = PicoScreenWidth * ratio;
        renderHeight = PicoScreenHeight * ratio;
    }

    u32 xOffset = (FB_WIDTH - renderWidth) / 2;
    u32 yOffset = (FB_HEIGHT - renderHeight) / 2;

    for (u32 y = 0; y < renderHeight; y ++)
    {
        for (u32 x = 0; x < renderWidth; x ++)
        {
            int picoX = (int)(x / ratio);
            int picoY = (int)(y / ratio);
            uint8_t c = getPixelNibble(picoX, picoY, picoFb);
            //uint8_t c = picoFb[x*128 + y];
            Color col = _paletteColors[screenPaletteMap[c]];

            u32 pos = (yOffset + y) * stride / sizeof(u32) + (xOffset + x);
            framebuf[pos] = RGBA8_MAXALPHA(col.Red, col.Green, col.Blue);

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
    return appletMainLoop();
}

vector<string> Host::listcarts(){
    vector<string> carts;

    DIR* dir = opendir("/p8carts");
    struct dirent *ent;
    std::string fullCartDir = "/p8carts/";

    if (dir) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            carts.push_back(fullCartDir + ent->d_name);
        }
        closedir (dir);
    }

    
    return carts;
}

const char* Host::logFilePrefix() {
    return "";
}

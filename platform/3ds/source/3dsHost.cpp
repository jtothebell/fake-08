#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;


#include "../../../source/host.h"
#include "../../../source/hostVmShared.h"
#include "../../../source/nibblehelpers.h"
#include "../../../source/PicoRam.h"

#define SCREEN_WIDTH 400;
#define SCREEN_HEIGHT 240;

#define SCREEN_2_WIDTH 320;
#define SCREEN_2_HEIGHT 240;

#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2

const int __3ds_TopScreenWidth = SCREEN_WIDTH;
const int __3ds_TopScreenHeight = SCREEN_HEIGHT;

const int __3ds_BottomScreenWidth = SCREEN_2_WIDTH;
const int __3ds_BottomScreenHeight = SCREEN_2_HEIGHT;


const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;

const int PicoFbLength = 128 * 64;


StretchOption stretch = StretchAndOverflow;
u64 last_time;
u64 now_time;
u64 frame_time;
double targetFrameTimeMs;

u32 currKDown;
u32 currKHeld;

Color* _paletteColors;
Bgr24Col _bgrColors[16];

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

	if (input & KEY_START){
		result |= P8_KEY_PAUSE;
	}

	if (input & KEY_SELECT){
		result |= P8_KEY_7;
	}

	return result;
}

void postFlipFunction(){
    gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
}


void init_fill_buffer(void *audioBuffer,size_t offset, size_t size) {

	u32 *dest = (u32*)audioBuffer;

	for (size_t i=0; i<size; i++) {
		dest[i] = 0;
	}

	DSP_FlushDataCache(audioBuffer,size);

}

bool audioInitialized = false;
u32 *audioBuffer;
u32 audioBufferSize;
ndspWaveBuf waveBuf[2];
bool fillBlock = false;
u32 currPos;


void audioCleanup(){
    audioInitialized = false;

    ndspExit();

    if(audioBuffer != nullptr) {
        linearFree(audioBuffer);
        audioBuffer = nullptr;
    }
}

void audioSetup(){
    if(R_FAILED(ndspInit())) {
        return;
    }

	//audio setup
	audioBufferSize = SAMPLESPERBUF * NUM_BUFFERS * sizeof(u32);
	audioBuffer = (u32*)linearAlloc(audioBufferSize);
	if(audioBuffer == nullptr) {
        audioCleanup();
        return;
    }
	

	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
	ndspChnSetRate(0, SAMPLERATE);
	ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);

	float mix[12];
	memset(mix, 0, sizeof(mix));
	mix[0] = 1.0;
	mix[1] = 1.0;
	ndspChnSetMix(0, mix);

	memset(waveBuf,0,sizeof(waveBuf));
	waveBuf[0].data_vaddr = &audioBuffer[0];
	waveBuf[0].nsamples = SAMPLESPERBUF;
	waveBuf[1].data_vaddr = &audioBuffer[SAMPLESPERBUF];
	waveBuf[1].nsamples = SAMPLESPERBUF;


	size_t stream_offset = 0;

	//not sure if this is necessary? if it is, memset might be better?
	init_fill_buffer(audioBuffer,stream_offset, SAMPLESPERBUF * 2);

	stream_offset += SAMPLESPERBUF;

	ndspChnWaveBufAdd(0, &waveBuf[0]);
	ndspChnWaveBufAdd(0, &waveBuf[1]);

	audioInitialized = true;
}



Host::Host() { }


void Host::oneTimeSetup(Color* paletteColors){
    osSetSpeedupEnable(true);

    audioSetup();

    gfxInitDefault();
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    _paletteColors = paletteColors;
    for(int i = 0; i < 16; i++){
        _bgrColors[i] = {
            _paletteColors[i].Blue,
            _paletteColors[i].Green,
            _paletteColors[i].Red
        };
    }
}

void Host::oneTimeCleanup(){
    audioCleanup();

	gfxExit();
}

void Host::setTargetFps(int targetFps){
    targetFrameTimeMs = 1000.0 / (double)targetFps;
}

void Host::changeStretch(){
    if (currKDown & KEY_R) {
        if (stretch == PixelPerfect) {
            stretch = StretchToFit;
        }
        else if (stretch == StretchToFit) {
            stretch = StretchAndOverflow;
        }
        else if (stretch == StretchAndOverflow) {
            stretch = PixelPerfect;
        }
    }
}

InputState_t Host::scanInput(){
    hidScanInput();

    currKDown = hidKeysDown();
    currKHeld = hidKeysHeld();

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


void Host::waitForTargetFps(){
    now_time = svcGetSystemTick();
    frame_time = now_time - last_time;
	last_time = now_time;

	double frameTimeMs = frame_time / CPU_TICKS_PER_MSEC;

	//sleep for remainder of time
	if (frameTimeMs < targetFrameTimeMs) {
		double msToSleep = targetFrameTimeMs - frameTimeMs;

		svcSleepThread(msToSleep * 1000 * 1000);

		last_time += CPU_TICKS_PER_MSEC * msToSleep;
	}
}


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap){
    int bgcolor = 0;
	uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	//clear whole top framebuffer
	memset(fb, bgcolor, __3ds_TopScreenWidth*__3ds_TopScreenHeight*3);

	//clear bottom buffer in case overflow rendering is being used
	uint8_t* fbb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(fbb, bgcolor, __3ds_BottomScreenWidth*__3ds_BottomScreenHeight*3);

    int x, y;

    //these could be combined to shorten this method
	if (stretch == PixelPerfect) {
		int xOffset = __3ds_TopScreenWidth / 2 - PicoScreenWidth / 2;
        int yOffset = __3ds_TopScreenHeight / 2 - PicoScreenHeight / 2;

       for(x = 0; x < 64; x++) {
            for(y = 0; y < 128; y++) {
                int x1 = x << 1;
                int x2 = x1 + 1;
                uint8_t lc = getPixelNibble(x1, y, picoFb);
                Bgr24Col lcol = _bgrColors[screenPaletteMap[lc]];

                int pixIdx = (((x1 + xOffset)*__3ds_TopScreenHeight)+ ((__3ds_TopScreenHeight - 1) - (y + yOffset)));

                ((Bgr24Col*)fb)[pixIdx] = lcol;

                uint8_t rc = getPixelNibble(x2, y, picoFb);
                Bgr24Col rcol = _bgrColors[screenPaletteMap[rc]];

                pixIdx = (((x2 + xOffset)*__3ds_TopScreenHeight)+ ((__3ds_TopScreenHeight - 1) - (y + yOffset)));

                ((Bgr24Col*)fb)[pixIdx] = rcol;
            }
        }
	}
	else if (stretch == StretchToFit) {
		double ratio = (double)__3ds_TopScreenHeight / (double)PicoScreenHeight;
        int stretchedWidth = PicoScreenWidth * ratio;

        int xOffset = __3ds_TopScreenWidth / 2 - stretchedWidth / 2;
        int yOffset = 0;
        
        for(x = 0; x < stretchedWidth; x++) {
            for(y = 0; y < __3ds_TopScreenHeight; y++) {
                int picoX = (int)(x / ratio);
                int picoY = (int)(y / ratio);
                uint8_t c = getPixelNibble(picoX, picoY, picoFb);
                Color col = _paletteColors[screenPaletteMap[c]];

                int pixIdx = (((x + xOffset)*__3ds_TopScreenHeight)+ ((__3ds_TopScreenHeight - 1) - (y + yOffset)))*3;

                fb[pixIdx + 0] = col.Blue;
                fb[pixIdx + 1] = col.Green;
                fb[pixIdx + 2] = col.Red;
            }
        }
	}
	else if (stretch == StretchAndOverflow) {
		//assume landscape, hardcoded double for now (3ds)
        int ratio = 2;
        int stretchedWidth = PicoScreenWidth * ratio;
        int stretchedHeight = PicoScreenHeight * ratio;

        int xOffset = __3ds_TopScreenWidth / 2 - stretchedWidth / 2;
        int yOffset = 0;

        for(x = 0; x < stretchedWidth; x++) {
            for(y = 0; y < __3ds_TopScreenHeight; y++) {
                int picoX = (int)(x / ratio);
                int picoY = (int)(y / ratio);
                uint8_t c = getPixelNibble(picoX, picoY, picoFb);
                Color col = _paletteColors[screenPaletteMap[c]];

                int pixIdx = (((x + xOffset)*__3ds_TopScreenHeight)+ ((__3ds_TopScreenHeight - 1) - (y + yOffset)))*3;

                fb[pixIdx + 0] = col.Blue;
                fb[pixIdx + 1] = col.Green;
                fb[pixIdx + 2] = col.Red;
            }
        }

        int overflowHeight = stretchedHeight - __3ds_TopScreenHeight;

        xOffset = __3ds_BottomScreenWidth / 2 - stretchedWidth / 2;
        yOffset = 0;

        for(x = 0; x < stretchedWidth; x++) {
            for(y = 0; y < overflowHeight; y++) {
                int picoX = (int)(x / ratio);
                int picoY = (int)((y + __3ds_TopScreenHeight) / ratio);
                uint8_t c = getPixelNibble(picoX, picoY, picoFb);
                Color col = _paletteColors[screenPaletteMap[c]];

                int pixIdx = (((x + xOffset)*__3ds_BottomScreenHeight)+ ((__3ds_BottomScreenHeight - 1) - (y + yOffset)))*3;

                fbb[pixIdx + 0] = col.Blue;
                fbb[pixIdx + 1] = col.Green;
                fbb[pixIdx + 2] = col.Red;
            }
        }
	}

    postFlipFunction();
}

bool Host::shouldFillAudioBuff(){
    return waveBuf[fillBlock].status == NDSP_WBUF_DONE;
}

void* Host::getAudioBufferPointer(){
    return waveBuf[fillBlock].data_pcm16;
}

size_t Host::getAudioBufferSize(){
    return waveBuf[fillBlock].nsamples;
}

void Host::playFilledAudioBuffer(){
    DSP_FlushDataCache(waveBuf[fillBlock].data_pcm16, waveBuf[fillBlock].nsamples);

	ndspChnWaveBufAdd(0, &waveBuf[fillBlock]);

	fillBlock = !fillBlock;
}

bool Host::shouldRunMainLoop(){
    return aptMainLoop();
}

vector<string> Host::listcarts(){
    vector<string> carts;

    //force to SD card root
    chdir("sdmc:/");

    DIR* dir = opendir("/p8carts");

    if (dir) {
        for(auto& p: fs::directory_iterator("/p8carts")){
            auto ext = p.path().extension().string();
            if (ext == ".p8" || ext == ".png"){
                carts.push_back(p.path().string());
            }
        }

        closedir(dir);
    }

    
    return carts;
}

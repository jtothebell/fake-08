#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>


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
#include "../../../source/PicoRam.h"
#include "../../../source/filehelpers.h"

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

u64 last_time;
u64 now_time;
u64 frame_time;
float targetFrameTimeMs;

u32 currKDown32;
u32 currKHeld32;
int touchLocationX;
int touchLocationY;
uint8_t mouseBtnState;

Color* _paletteColors;
uint16_t _rgb565Colors[144];
uint32_t _rgba8Colors[144];
Audio* _audio;

static C2D_Image pico_image;

C3D_Tex *pico_tex;
Tex3DS_SubTexture *pico_subtex;

C3D_RenderTarget* topTarget;
C3D_RenderTarget* bottomTarget;

u16* pico_pixel_buffer;

const GPU_TEXCOLOR texColor = GPU_RGB565;

#define CLEAR_COLOR 0xFF000000
#define BYTES_PER_PIXEL 2
size_t pico_pixel_buffer_size = 128*128*BYTES_PER_PIXEL;


int topXOffset = 0;
int topYOffset = 8;
int topSubTexWidth = 256;
int topSubTexHeight = 256;

int bottomXOffset = 0;
int bottomYOffset = -232;
int bottomSubTexWidth = 256;
int bottomSubTexHeight = 256;

float screenModeScaleX = 1.0f;
float screenModeScaleY = 1.0f;
float screenModeAngle = 0;
int flipHorizontal = 1;
int flipVertical = 1;


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

	return result;
}

void setRenderParamsFromStretch(StretchOption stretch) {
    if (stretch == StretchToFit) {
        topXOffset = 0;
        topYOffset = 0;
        topSubTexWidth = 240;
        topSubTexHeight = 240;
        
        bottomXOffset = 0;
        bottomYOffset = 0;
        bottomSubTexWidth = 0;
        bottomSubTexHeight = 0;
    }
    else if (stretch == StretchAndOverflow) {
        topXOffset = 0;
        topYOffset = 8;
        topSubTexWidth = 256;
        topSubTexHeight = 256;

        bottomXOffset = 0;
        bottomYOffset = -232;
        bottomSubTexWidth = 256;
        bottomSubTexHeight = 256;
    }
    else if (stretch == AltScreenPixelPerfect) {
        topXOffset = 0;
        topYOffset = 0;
        topSubTexWidth = 0;
        topSubTexHeight = 0;

        bottomXOffset = 0;
        bottomYOffset = 0;
        bottomSubTexWidth = 128;
        bottomSubTexHeight = 128;
    }
    else if (stretch == AltScreenStretch) {
        topXOffset = 0;
        topYOffset = 0;
        topSubTexWidth = 0;
        topSubTexHeight = 0;

        bottomXOffset = 0;
        bottomYOffset = 0;
        bottomSubTexWidth = 240;
        bottomSubTexHeight = 240;
    }
    else {
        topXOffset = 0;
        topYOffset = 0;
        topSubTexWidth = 128;
        topSubTexHeight = 128;
        
        bottomXOffset = 0;
        bottomYOffset = 0;
        bottomSubTexWidth = 0;
        bottomSubTexHeight = 0;
    }
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




Host::Host() {
    _logFilePrefix = "sdmc:/3ds/fake08/";

    _cartDirectory = "/p8carts/";

    struct stat st = {0};

    int res = chdir("sdmc:/");
    
    if (res == 0 && stat("3ds", &st) == -1) {
        res = mkdir("3ds", 0777);
    }

    if (res == 0 && stat(_logFilePrefix.c_str(), &st) == -1) {
        res = mkdir(_logFilePrefix.c_str(), 0777);
    }

    string cartdatadir = _logFilePrefix + "cdata";
    if (res == 0 && stat(cartdatadir.c_str(), &st) == -1) {
        res = mkdir(cartdatadir.c_str(), 0777);
    }
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
    osSetSpeedupEnable(true);

    stretch = StretchAndOverflow;

    _audio = audio;
    audioSetup();

    gfxInitDefault();
    //C3D_Init(C3D_DEFAULT_CMDBUF_SIZE); default is 0x40000
    C3D_Init(0x10000);
	//C2D_Init(C2D_DEFAULT_MAX_OBJECTS); //4096
    C2D_Init(32); //need very few objects? this probably doesn't really help perf
	C2D_Prepare();

    topTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    /*
    topTarget = C3D_RenderTargetCreate(GSP_SCREEN_WIDTH, GSP_SCREEN_HEIGHT_TOP, GPU_RB_RGBA8, GPU_RB_DEPTH16);
	if (topTarget) {
		C3D_RenderTargetSetOutput(topTarget, GFX_TOP, GFX_LEFT,
			GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) |
			GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) |
			GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));
    }
    */
    bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

    currKDown32 = 0;
    currKHeld32 = 0;

    _paletteColors = paletteColors;
    for(int i = 0; i < 144; i++){
        _rgba8Colors[i] = 0xFF | (_paletteColors[i].Blue << 8) | (_paletteColors[i].Green << 16) | (_paletteColors[i].Red << 24);
        _rgb565Colors[i] = (((_paletteColors[i].Red & 0xf8)<<8) + ((_paletteColors[i].Green & 0xfc)<<3)+(_paletteColors[i].Blue>>3));
    }

    pico_tex = (C3D_Tex*)linearAlloc(sizeof(C3D_Tex));
	//people on homebrew discord said should use this
	C3D_TexInitVRAM(pico_tex, 128, 128, texColor);
	//can't tell a difference, but I'd expect vram to be better? maybe not if transferring often
	//C3D_TexInit(pico_tex, 128, 128, GPU_RGBA8);
	C3D_TexSetFilter(pico_tex, GPU_NEAREST, GPU_NEAREST);

	pico_subtex = (Tex3DS_SubTexture*)linearAlloc(sizeof(Tex3DS_SubTexture));
	pico_subtex->width = 128;
	pico_subtex->height = 128;
	pico_subtex->left = 0.0f;
	pico_subtex->top = 1.0f;
	pico_subtex->right = 1.0f;
	pico_subtex->bottom = 0.0f;

	pico_image.tex = pico_tex;
	pico_image.subtex = pico_subtex;

	pico_pixel_buffer = (u16*)linearAlloc(pico_pixel_buffer_size);

    loadSettingsIni();

    setRenderParamsFromStretch(stretch);

    if (stretch == AltScreenPixelPerfect) {
        mouseOffsetX = (__3ds_BottomScreenWidth - PicoScreenWidth) / 2;
        mouseOffsetY = (__3ds_BottomScreenHeight - PicoScreenHeight) / 2;
        scaleX = 1.0;
        scaleY = 1.0;
    }
    else{
        mouseOffsetX = (__3ds_BottomScreenWidth - __3ds_BottomScreenHeight) / 2;
        mouseOffsetY = 0;
        scaleX = 0.53;
        scaleY = 0.53;
    }
}

void Host::oneTimeCleanup(){
    saveSettingsIni();

    audioCleanup();

    C3D_TexDelete(pico_tex);

	linearFree(pico_tex);
	linearFree(pico_subtex);
    linearFree(pico_pixel_buffer);

    C2D_Fini();
	C3D_Fini();
	gfxExit();
}

void Host::setTargetFps(int targetFps){
    targetFrameTimeMs = 1000.0 / (float)targetFps;
}

void Host::changeStretch(){
    if (currKDown32 & KEY_SELECT) {
        if (stretch == PixelPerfect) {
            stretch = StretchToFit;
            mouseOffsetX = (__3ds_BottomScreenWidth - __3ds_BottomScreenHeight) / 2;
            mouseOffsetY = 0;
            scaleX = 0.53;
            scaleY = 0.53;
        }
        else if (stretch == StretchToFit) {
            stretch = StretchAndOverflow;
            mouseOffsetX = (__3ds_BottomScreenWidth - __3ds_BottomScreenHeight) / 2;
            mouseOffsetY = 0;
            scaleX = 0.53;
            scaleY = 0.53;
        }
        else if (stretch == StretchAndOverflow) {
            stretch = AltScreenPixelPerfect;
            mouseOffsetX = (__3ds_BottomScreenWidth - PicoScreenWidth) / 2;
            mouseOffsetY = (__3ds_BottomScreenHeight - PicoScreenHeight) / 2;
            scaleX = 1.0;
            scaleY = 1.0;
        }
        else if (stretch == AltScreenPixelPerfect) {
            stretch = AltScreenStretch;
            mouseOffsetX = (__3ds_BottomScreenWidth - __3ds_BottomScreenHeight) / 2;
            mouseOffsetY = 0;
            scaleX = 0.53;
            scaleY = 0.53;
        }
        else {
            stretch = PixelPerfect;
            mouseOffsetX = (__3ds_BottomScreenWidth - __3ds_BottomScreenHeight) / 2;
            mouseOffsetY = 0;
            scaleX = 0.53;
            scaleY = 0.53;
        }

        setRenderParamsFromStretch(stretch);

    }
}

InputState_t Host::scanInput(){
    hidScanInput();

    currKDown32 = hidKeysDown();
    currKHeld32 = hidKeysHeld();

    touchPosition touch;

	//Read the touch screen coordinates
	hidTouchRead(&touch);

    if (touch.px > 0 && touch.py > 0) {
        touchLocationX = (touch.px - mouseOffsetX) * scaleX;
        touchLocationY = (touch.py - mouseOffsetY) * scaleY;
        mouseBtnState = 1;
    }
    else {
        mouseBtnState = 0;
    }

    return InputState_t {
        ConvertInputToP8(currKDown32),
        ConvertInputToP8(currKHeld32),
        (int16_t)touchLocationX,
        (int16_t)touchLocationY,
        mouseBtnState
    };
}

bool Host::shouldQuit() {
    bool lpressed = currKHeld32 & KEY_L;
	bool rpressed = currKDown32 & KEY_R;

	return lpressed && rpressed;
}


void Host::waitForTargetFps(){
    now_time = svcGetSystemTick();
    frame_time = now_time - last_time;
	last_time = now_time;

	float frameTimeMs = frame_time / CPU_TICKS_PER_MSEC;

	//sleep for remainder of time
	if (frameTimeMs < targetFrameTimeMs) {
		float msToSleep = targetFrameTimeMs - frameTimeMs;

		svcSleepThread(msToSleep * 1000 * 1000);

		last_time += CPU_TICKS_PER_MSEC * msToSleep;
	}
}


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t drawMode){
    size_t pixIdx = 0;

    for (pixIdx = 0; pixIdx < pico_pixel_buffer_size; pixIdx++){
        pico_pixel_buffer[pixIdx] = _rgb565Colors[screenPaletteMap[getPixelNibble(pixIdx % 128, pixIdx / 128, picoFb)]];
    }

    //not sure if this is necessary?
    GSPGPU_FlushDataCache(pico_pixel_buffer, pico_pixel_buffer_size);

	C3D_SyncDisplayTransfer(
        (u32*)pico_pixel_buffer, GX_BUFFER_DIM(128, 128),
        (u32*)(pico_tex->data), GX_BUFFER_DIM(128, 128),
		(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
        GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB565) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB565) |
        GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
    );

    screenModeScaleX = 1.0f;
    screenModeScaleY = 1.0f;
    screenModeAngle = 0;
    flipHorizontal = 1;
    flipVertical = 1;

    switch(drawMode){
        case 1:
            screenModeScaleX = 2.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 0;
            flipHorizontal = 1;
            flipVertical = 1;
            break;
        case 2:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 2.0f;
            screenModeAngle = 0;
            flipHorizontal = 1;
            flipVertical = 1;
            break;
        case 3:
            screenModeScaleX = 2.0f;
            screenModeScaleY = 2.0f;
            screenModeAngle = 0;
            flipHorizontal = 1;
            flipVertical = 1;
            break;
        //todo: mirroring- not sure how to do this?
        //case 5,6,7

        case 129:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 0;
            flipHorizontal = -1;
            flipVertical = 1;
            break;
        case 130:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 0;
            flipHorizontal = 1;
            flipVertical = -1;
            break;
        case 131:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 0;
            flipHorizontal = -1;
            flipVertical = -1;
            break;
        case 133:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 1.5707963267949f; // pi / 2 (90 degrees)
            flipHorizontal = 1;
            flipVertical = 1;
            break;
        case 134:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 3.1415926535898f; //pi (180 degrees)
            flipHorizontal = 1;
            flipVertical = 1;
            break;
        case 135:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 4.7123889803847f; // pi * 3 / 2 (270 degrees)
            flipHorizontal = 1;
            flipVertical = 1;
            break;
        default:
            screenModeScaleX = 1.0f;
            screenModeScaleY = 1.0f;
            screenModeAngle = 0;
            flipHorizontal = 1;
            flipVertical = 1;
            break;
    }

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		C2D_TargetClear(topTarget, CLEAR_COLOR);
		C2D_SceneBegin(topTarget);

        if (topSubTexWidth > 0 && topSubTexHeight > 0) {
            pico_subtex->width = topSubTexWidth;
            pico_subtex->height = topSubTexHeight;
            pico_subtex->left = 0.0f / screenModeScaleX;
            //top and bottom are inverted from what I usually expect
            pico_subtex->top = 1.0f - (0.0f / screenModeScaleY);
            pico_subtex->right = 1.0f / screenModeScaleX;
            pico_subtex->bottom = 1.0f - (1.0f / screenModeScaleY);

            /*
            C2D_DrawImageAt(
                pico_image,
                topXOffset,
                topYOffset,
                .5,
                NULL,
                topScaleX * screenModeScaleX,
                topScaleY * screenModeScaleY);
                */

            //drawimageatrotated coord is center of image, drawimageage coord is top left
            C2D_DrawImageAtRotated(
                pico_image,
                200 + topXOffset,
                120 + topYOffset,
                .5,
                screenModeAngle,
                NULL,
                flipHorizontal,
                flipVertical);
        }

        
        C2D_TargetClear(bottomTarget, CLEAR_COLOR);
        C2D_SceneBegin(bottomTarget);

        if (bottomSubTexWidth > 0 && bottomSubTexHeight > 0) {
            pico_subtex->width = bottomSubTexWidth;
            pico_subtex->height = bottomSubTexHeight;
            pico_subtex->left = 0.0f / screenModeScaleX;
            //top and bottom are inverted from what I usually expect
            pico_subtex->top = 1.0f - (0.0f / screenModeScaleY);
            pico_subtex->right = 1.0f / screenModeScaleX;
            pico_subtex->bottom = 1.0f - (1.0f / screenModeScaleY);

/*
            C2D_DrawImageAt(
                pico_image,
                bottomXOffset,
                bottomYOffset,
                .5,
                NULL,
                1,
                1);
                */

            C2D_DrawImageAtRotated(
                pico_image,
                160 + bottomXOffset,
                120 + bottomYOffset,
                .5,
                screenModeAngle,
                NULL,
                flipHorizontal,
                flipVertical);
        }

		C2D_Flush();

	C3D_FrameEnd(0);
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
    struct dirent *ent;

    if (dir) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            if (isCartFile(ent->d_name)){
                carts.push_back(_cartDirectory + ent->d_name);
            }
        }
        closedir (dir);
    }
    
    return carts;
}

const char* Host::logFilePrefix() {
    return "";
}

std::string Host::customBiosLua() {
    return "";
}

std::string Host::getCartDirectory() {
    return _cartDirectory;
}

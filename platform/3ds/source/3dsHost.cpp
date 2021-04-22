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
size_t pico_rgba_buffer_size = 128*128*BYTES_PER_PIXEL;

u32 clrRec1;

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
        std::string customBiosLua) {}


void Host::oneTimeSetup(Color* paletteColors, Audio* audio){
    osSetSpeedupEnable(true);

    stretch = StretchAndOverflow;

    _audio = audio;
    audioSetup();

    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

    topTarget = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottomTarget = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
    
    last_time = 0;
    now_time = 0;
    frame_time = 0;
    targetFrameTimeMs = 0;

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

	pico_pixel_buffer = (u16*)linearAlloc(pico_rgba_buffer_size);

    clrRec1 = C2D_Color32(0x9A, 0x6C, 0xB9, 0xFF);

    loadSettingsIni();

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
    if (currKDown32 & KEY_R) {
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


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap){
    int pixIdx = 0, x = 0, y = 0;

    for (pixIdx = 0; pixIdx < pico_rgba_buffer_size; pixIdx++){
        x = pixIdx % 128;
        y = pixIdx / 128;
        //uint8_t lc = getPixelNibble(x, y, picoFb);
        //uint16_t lcol = _rgb565Colors[screenPaletteMap[lc]];
        //u32 lcol = _rgba8Colors[screenPaletteMap[lc]];
        pico_pixel_buffer[pixIdx] = _rgb565Colors[screenPaletteMap[getPixelNibble(x, y, picoFb)]];
    }

    /*
    for(x = 0; x < 64; x++) {
        for(y = 0; y < 128; y++) {
            int x1 = x << 1;
            int x2 = x1 + 1;
            uint8_t lc = getPixelNibble(x1, y, picoFb);
            //uint16_t lcol = _rgb565Colors[screenPaletteMap[lc]];
            u32 lcol = _rgba8Colors[screenPaletteMap[lc]];

            pico_pixel_buffer[pixIdx++] = lcol;

            uint8_t rc = getPixelNibble(x2, y, picoFb);
            //uint16_t rcol = _rgb565Colors[screenPaletteMap[rc]];
            u32 rcol = _rgba8Colors[screenPaletteMap[rc]];

            pico_pixel_buffer[pixIdx++] = rcol;
        }
    }
    */

    GSPGPU_FlushDataCache(pico_pixel_buffer, pico_rgba_buffer_size);

	C3D_SyncDisplayTransfer(
        (u32*)pico_pixel_buffer, GX_BUFFER_DIM(128, 128),
        (u32*)(pico_tex->data), GX_BUFFER_DIM(128, 128),
		(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(1) | GX_TRANSFER_RAW_COPY(0) |
        GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB565) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB565) |
        GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))
    );

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

		C2D_TargetClear(topTarget, CLEAR_COLOR);
		C2D_SceneBegin(topTarget);

		C2D_DrawRectangle(0, 128, 0, 64, 64, clrRec1, clrRec1, clrRec1, clrRec1);

		C2D_DrawImageAt(pico_image, 72, 0, .5, NULL, 2.0f, 2.0f);

		//C2D_Flush();

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

std::string Host::customBiosLua() {
    return "";
}

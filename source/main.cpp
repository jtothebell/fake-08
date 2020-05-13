

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <math.h>

#include <string>

#include "graphics.h"
#include "console.h"
#include "logger.h"

#include "input.h"

//this has the macro _TEST defined in it
#include "tests/test_base.h"

#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 30)
#define NUM_BUFFERS 2


const int ScreenWidth = 400;
const int ScreenHeight = 240;

const int BottomScreenWidth = 320;
const int BottomScreenHeight = 240;

StretchOption stretch = PixelPerfect;

void ChangeStretch() {
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

//3ds specific helper function
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

void clear3dsFrameBuffer() {
	#if _TEST
	int bgcolor = 255;
	#else
	int bgcolor = 0;
	#endif
	uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	//clear whole top framebuffer
	memset(fb, bgcolor, ScreenHeight*ScreenWidth*3);

	//clear top 16 pixels of bottom buffer in case overflow rendering is being used
	uint8_t* fbb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(fbb, bgcolor, BottomScreenHeight*BottomScreenWidth*3);

}

//3ds specific helper function
void postFlip3dsFunction() {
	gfxFlushBuffers();
	gfxSwapBuffers();
	gspWaitForVBlank();
}

//----------------------------------------------------------------------------
void init_fill_buffer(void *audioBuffer,size_t offset, size_t size) {
//----------------------------------------------------------------------------

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

void audioCleanup() {
    audioInitialized = false;

    ndspExit();

    if(audioBuffer != nullptr) {
        linearFree(audioBuffer);
        audioBuffer = nullptr;
    }
}

void audioSetup() {
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


int main(int argc, char* argv[])
{
	u64 last_time = 0, now_time = 0, frame_time = 0;

	audioSetup();

	Logger::Initialize();
	Logger::Write("created Logger\n");
	gfxInitDefault();
	Logger::Write("gfxInitDefault()\n");

	//use new 3ds cpu if we can
	osSetSpeedupEnable(true);	

	Logger::Write("initializing Console\n");
	Console *console = new Console();
	Logger::Write("initialized console\n");

	//test or not both hardcoded to loading test cart as of now
	#if _TEST
	consoleInit(GFX_BOTTOM, NULL);

	Logger::Write("Loading cart\n");
	console->LoadCart("testcart.p8");
	Logger::Write("Cart Loaded\n");

	#else
	console->LoadCart("lilking.p8");
	#endif
	
	// Main loop
	Logger::Write("Starting main loop\n");

	uint8_t targetFps = console->GetTargetFps();
	double targetFrametimeMs = 1000.0 / (double)targetFps;

	std::function<void()> clearFb = clear3dsFrameBuffer;
	std::function<void()> postFlip = postFlip3dsFunction;

	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		now_time = svcGetSystemTick();
     	frame_time = now_time - last_time;
		last_time = now_time;

		double frameTimeMs = frame_time / CPU_TICKS_PER_MSEC;

		consoleClear();
		#if _TEST

		printf("\n"); //make space for overflow if needed
		printf("svcGetSystemTick(): %lld \n", now_time);
		printf("frame time (ticks): %lld \n", frame_time);
		printf("frame time (ms): %f \n", frameTimeMs);
		printf("target fps: %d \n", targetFps);
		printf("target frame time (ms): %f \n", targetFrametimeMs);
		#endif

		//sleep for remainder of time
		if (frameTimeMs < targetFrametimeMs) {
			double msToSleep = targetFrametimeMs - frameTimeMs;
			
			#if _TEST
			printf("sleeping for : %f ms\n", msToSleep);
			#endif

			svcSleepThread(msToSleep * 1000 * 1000);

			last_time += CPU_TICKS_PER_MSEC * msToSleep;
		}
		
		
		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		bool lpressed = kHeld & KEY_L;
		bool rpressed = kHeld & KEY_R;

		if (lpressed && rpressed) break; // break in order to return to hbmenu

		if (kDown & KEY_R) {
			ChangeStretch();
		}

		uint8_t p8kDown = ConvertInputToP8(kDown);
		uint8_t p8kHeld = ConvertInputToP8(kHeld);


		size_t stream_offset = 0;

		//_update();
		

		//uint8_t* fb_b = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		//clear whole framebuffer
		//memset(fb_b, bgcolor, 240*320*3);

		//cart draw
		//_draw();

		console->UpdateAndDraw(frame_time, clear3dsFrameBuffer, p8kDown, p8kHeld);

		//send pico 8 screen to framebuffer, then call the function to flush and swap buffers, and wait for vblank
		
		uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);

		if (stretch == PixelPerfect) {
			console->FlipBuffer_PP(fb, ScreenWidth, ScreenHeight, postFlip);
		}
		else if (stretch == StretchToFit) {
			console->FlipBuffer_STF(fb, ScreenWidth, ScreenHeight, postFlip);
		}
		else if (stretch == StretchAndOverflow) {
			uint8_t* fbb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);

			console->FlipBuffer_SAO(
				fb, ScreenWidth, ScreenHeight,
				fbb, BottomScreenWidth, BottomScreenHeight,
				postFlip);
		}

		if (waveBuf[fillBlock].status == NDSP_WBUF_DONE) {

			console->FillAudioBuffer(waveBuf[fillBlock].data_pcm16, stream_offset, waveBuf[fillBlock].nsamples);
			
			DSP_FlushDataCache(waveBuf[fillBlock].data_pcm16, waveBuf[fillBlock].nsamples);

			ndspChnWaveBufAdd(0, &waveBuf[fillBlock]);
			stream_offset += waveBuf[fillBlock].nsamples;

			fillBlock = !fillBlock;
		}

	}


	Logger::Write("Turning off console and exiting logger\n");
	console->TurnOff();
	delete console;
	Logger::Exit();

	audioCleanup();
	gfxExit();

	return 0;
}



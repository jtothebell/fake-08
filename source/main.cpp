

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <string>

#include "graphics.h"
#include "console.h"
#include "logger.h"

#include "input.h"

//this has the macro _TEST defined in it
#include "tests/test_base.h"

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

int main(int argc, char* argv[])
{
	int frames = 0; 
	Logger::Initialize();
	Logger::Write("created Logger\n");
	gfxInitDefault();
	Logger::Write("gfxInitDefault()\n");

	Logger::Write("initializing Console\n");
	Console console;
	Logger::Write("initialized console\n");

	//test or not both hardcoded to loading test cart as of now
	#if _TEST
	int bgcolor = 255;
	consoleInit(GFX_BOTTOM, NULL);

	Logger::Write("Loading cart\n");
	console.LoadCart("testcart.p8");
	Logger::Write("Cart Loaded\n");

	#else
	console.LoadCart("testcart.p8");
	int bgcolor = 0;
	#endif
	
	// Main loop
	Logger::Write("Starting main loop\n");

	while (aptMainLoop())
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();

		bool lpressed = kHeld & KEY_L;
		bool rpressed = kHeld & KEY_R;

		if (lpressed && rpressed) break; // break in order to return to hbmenu

		uint8_t p8kDown = ConvertInputToP8(kDown);
		uint8_t p8kHeld = ConvertInputToP8(kHeld);

		//_update();
		
		uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		//clear whole top framebuffer
		memset(fb, bgcolor, 240*400*3);

		//uint8_t* fb_b = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		//clear whole framebuffer
		//memset(fb_b, bgcolor, 240*320*3);

		//cart draw
		//_draw();

		console.UpdateAndDraw(frames, p8kDown, p8kHeld);

		//send pico 8 screen to framebuffer
		console.FlipBuffer(fb);

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		gspWaitForVBlank();

    	frames++;
	}


	Logger::Write("Turning off console and exiting logger\n");
	console.TurnOff();
	Logger::Exit();
	gfxExit();
	return 0;
}



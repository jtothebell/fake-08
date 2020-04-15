

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#include <string>

#include "graphics.h"
#include "fakecart.h"
#include "console.h"

//this has the macro _TEST defined in it
#include "tests/test_base.h"

int main(int argc, char* argv[])
{
	int frames = 0; 
	gfxInitDefault();

	Console console;

	//test or not both hardcoded to loading test cart as of now
	#if _TEST
	int bgcolor = 255;
	consoleInit(GFX_BOTTOM, NULL);

	console.LoadCart("testcart.p8");

	#else
	loadcart("testcart.p8");
	int bgcolor = 0;
	#endif
	
	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();

		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u32 kDown = hidKeysDown();

		if (kDown & KEY_START) break; // break in order to return to hbmenu

		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		_update();
		
		uint8_t* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		//clear whole top framebuffer
		memset(fb, bgcolor, 240*400*3);

		//uint8_t* fb_b = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
		//clear whole framebuffer
		//memset(fb_b, bgcolor, 240*320*3);

		//cart draw
		_draw();

		//send pico 8 screen to framebuffer
		flipBuffer(fb);

    	frames++;
	}

	gfxExit();
	return 0;
}

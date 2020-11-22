#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>

#include "vm.h"
#include "logger.h"
#include "host.h"
#include "hostVmShared.h"



int main(int argc, char* argv[])
{
	Logger::Initialize();

	Host *host = new Host();
	Vm *vm = new Vm(host);
	
	host->oneTimeSetup(vm->GetPaletteColors());
	
	Logger::Write("initialized Vm and host\n");

	Logger::Write("Setting cart list on vm\n");
	vm->SetCartList(host->listcarts());

	Logger::Write("Loading Bios cart\n");
	vm->LoadBiosCart();
	Logger::Write("Bios Cart Loaded\n");

	// Main loop
	Logger::Write("Starting main loop\n");

	while (host->shouldRunMainLoop())
	{
		int targetFps = vm->GetTargetFps();
		//shouldn't need to set this every frame
		host->setTargetFps(targetFps);

		//is this better at the end of the loop?
		host->waitForTargetFps();

		if (host->shouldQuit()) break; // break in order to return to hbmenu
		//this should probably be handled just in the host class
		host->changeStretch();

		//update buttons needs to be callable from the cart, and also flip
		//it should update call the pico part of scanInput and set the values in memory
		//then we don't need to pass them in here
		vm->UpdateAndDraw();

		uint8_t* picoFb = vm->GetPicoInteralFb();
		uint8_t* screenPaletteMap = vm->GetScreenPaletteMap();

		host->drawFrame(picoFb, screenPaletteMap);

		if (host->shouldFillAudioBuff()) {
			vm->FillAudioBuffer(host->getAudioBufferPointer(), 0, host->getAudioBufferSize());

			host->playFilledAudioBuffer();
		}
	}


	Logger::Write("Turning off vm and exiting logger\n");
	vm->CloseCart();
	delete vm;
	
	Logger::Exit();

	host->oneTimeCleanup();
	delete host;

	return 0;
}



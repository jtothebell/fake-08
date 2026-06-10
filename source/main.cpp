#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include <string>

#include "vm.h"
#include "logger.h"
#include "host.h"
#include "hostVmShared.h"

#if __VITA__

#include <vitasdk.h>

#endif

bool g_runAndExit = false;

int main(int argc, char* argv[])
{
	//default to full resolution
	int windowWidth = 0;
	int windowHeight = 0;
	int exitFrames = 1;
	int opt;

	while ((opt = getopt(argc, argv, "w:h:xf:")) != -1) {
    	switch (opt) {
    		case 'w': windowWidth = (int)strtol(optarg, NULL, 10); break;
    		case 'h': windowHeight = (int)strtol(optarg, NULL, 10); break;
    		case 'x': g_runAndExit = true; break;
    		case 'f':
    			exitFrames = (int)strtol(optarg, NULL, 10);
    			if (exitFrames < 0) {
    				fprintf(stderr, "frame count must be >= 0\n");
    				return 2;
    			}
    			break;
    		default:
    			fprintf(stderr, "Usage: %s [-x] [-f frames] [-w width] [-h height] [cart.p8]\n", argv[0]);
    			return 2;
        }
    }

	if (!g_runAndExit) {
		printf("windowWidth: %d\n", windowWidth);
		printf("windowHeight: %d\n", windowHeight);
	}

	Host *host = new Host(windowWidth, windowHeight);
	PicoRam *memory = new PicoRam();
	memory->Reset();
	Audio *audio = new Audio(memory);

	Logger_Initialize(host->logFilePrefix());
	Logger_Write("initializing Vm\n");
	Vm *vm = new Vm(host, memory, nullptr, nullptr, audio);
	

	host->setUpPaletteColors();
	host->oneTimeSetup(audio);
	host->setTargetFps(60);
	
	#if LOAD_PACK_INS

	host->unpackCarts();
	#endif

	
	
	Logger_Write("initialized Vm and host\n");

	Logger_Write("Setting cart list on vm\n");
	
	vm->SetCartList(host->listcarts());

	bool loadCart = false;
	char* cart;

	#if __VITA__
	char boot_params[1024];
	sceAppMgrGetAppParam(boot_params);
	if (strstr(boot_params,"psgm:play") && strstr(boot_params, "&param=")) {
		loadCart = true;
		cart = strstr(boot_params, "&param=") + 7;
	}
	#else
	int index;
	for (index = optind; index < argc; index++) {
		cart = argv[index];
		loadCart = true;
	}
	#endif

	if (g_runAndExit && !loadCart) {
		fprintf(stderr, "%s: -x requires a cart path\n", argv[0]);
		delete vm;
		delete host;
		Logger_Exit();
		return 2;
	}
	
	if (loadCart){
		Logger_Write("Loading arg cart \n");
		vm->LoadCart(cart, !g_runAndExit);
	}
	else {
		Logger_Write("Loading bios cart\n");
		vm->LoadBiosCart();
	}

	vm->vm_run();

	if (g_runAndExit) {
		for (int frame = 0; frame < exitFrames; ++frame) {
			if (!vm->Step()) {
				break;
			}
		}

		if (!vm->GetBiosError().empty()) {
			fprintf(stderr, "cart error: %s\n", vm->GetBiosError().c_str());
			vm->CloseCart();
			host->oneTimeCleanup();
			delete vm;
			delete host;
			Logger_Exit();
			return 1;
		}
	}
	else {
		// Main loop
		Logger_Write("Starting main loop\n");
		vm->GameLoop();
	}

	Logger_Write("Turning off vm and exiting logger\n");
	vm->CloseCart();

	Logger_Write("calling one time cleanup\n");
	host->oneTimeCleanup();

	Logger_Write("deleting vm\n");
	delete vm;
	Logger_Write("deleting host\n");
	delete host;
	
	Logger_Write("exiting logger\n");
	Logger_Exit();

	return 0;
}


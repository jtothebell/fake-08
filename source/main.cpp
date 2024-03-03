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



int main(int argc, char* argv[])
{
	//default to full resolution
	int windowWidth = 0;
	int windowHeight = 0;
	int opt;

	while ((opt = getopt(argc, argv, "w:h:")) != -1) {
    	switch (opt) {
    		case 'w': windowWidth = (int)strtol(optarg, NULL, 10); break;
    		case 'h': windowHeight = (int)strtol(optarg, NULL, 10); break;
        }
    }

	printf("windowWidth: %d\n", windowWidth);
	printf("windowHeight: %d\n", windowHeight);

	Host *host = new Host(windowWidth, windowHeight);
	PicoRam *memory = new PicoRam();
	memory->Reset();
	Audio *audio = new Audio(memory);

	Logger_Initialize(host->logFilePrefix());
	Logger_Write("initializing Vm\n");
	Vm *vm = new Vm(host, memory, nullptr, nullptr, audio);
	

	host->setUpPaletteColors();
	host->oneTimeSetup(audio);
	
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

	
	if (loadCart){
		Logger_Write("Loading arg cart \n");
		vm->LoadCart(cart);
	}
	// else {
	// 	vm->LoadBiosCart();
	// }
	//Logger_Write("Bios Cart Loaded\n");

	// Main loop
	Logger_Write("Starting main loop\n");

	vm->GameLoop();

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



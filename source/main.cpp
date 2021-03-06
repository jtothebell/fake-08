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
	Host *host = new Host();
	PicoRam *memory = new PicoRam();
	Audio *audio = new Audio(memory);

	Logger_Initialize(host->logFilePrefix());
	
	Vm *vm = new Vm(host, memory, nullptr, nullptr, audio);
	
	host->oneTimeSetup(vm->GetPaletteColors(), audio);
	
	Logger_Write("initialized Vm and host\n");

	Logger_Write("Setting cart list on vm\n");
	vm->SetCartList(host->listcarts());

	Logger_Write("Loading Bios cart\n");
	vm->LoadBiosCart();
	Logger_Write("Bios Cart Loaded\n");

	// Main loop
	Logger_Write("Starting main loop\n");

	vm->GameLoop();

	Logger_Write("Turning off vm and exiting logger\n");
	vm->CloseCart();
	Logger_Write("deleting vm\n");
	delete vm;

	Logger_Write("calling one time cleanup\n");
	host->oneTimeCleanup();
	Logger_Write("deleting host\n");
	delete host;
	
	Logger_Write("exiting logger\n");
	Logger_Exit();

	return 0;
}



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

	vm->GameLoop();

	Logger::Write("Turning off vm and exiting logger\n");
	vm->CloseCart();
	delete vm;
	
	Logger::Exit();

	host->oneTimeCleanup();
	delete host;

	return 0;
}



#pragma once

#include <string>
#include "graphics.h"
#include "vm.h"
#include "Audio.h"
#include "PicoRam.h"


void initPrintHelper(PicoRam* memory, Graphics* graphics, Vm* vm, Audio* audio);

int print(std::string str);

int print(std::string str, int x, int y);

int print(std::string str, int x, int y, uint8_t c);
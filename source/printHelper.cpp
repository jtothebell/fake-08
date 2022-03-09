#include <string>

#include "printHelper.h"

#include <string>
#include "graphics.h"
#include "vm.h"
#include "Audio.h"

PicoRam* _ph_mem;
Graphics* _ph_graphics;
Vm* _ph_vm;
Audio* _ph_audio;

void initPrintHelper(PicoRam* memory, Graphics* graphics, Vm* vm, Audio* audio) {
    _ph_mem = memory;
    _ph_graphics = graphics;
    _ph_vm = vm;
    _ph_audio = audio;
}

uint8_t p0CharToNum(uint8_t hexChar){
	uint8_t num = 0;
	if (hexChar > 47 && hexChar < 58){
		num = hexChar - 48;
	}
	if (hexChar > 96) {
		num = hexChar - 87;
	}

	return num;
}

int pow2(int power){
    switch(power){
        case 0:
            return 1;
            break;
        case 1:
            return 2;
            break;
        case 2:
            return 4;
            break;
        case 3:
            return 8;
            break;
        case 4:
            return 16;
            break;
        case 5:
            return 32;
            break;
        case 6:
            return 64;
            break;
        case 7:
            return 128;
            break;
        case 8:
            return 255;
            break;
    }
    return 0;
}

int print(std::string str) {
	int result = print(str, _ph_mem->drawState.text_x, _ph_mem->drawState.text_y);

	return result;
}

int print(std::string str, int x, int y) {
	return print(str, x, y, _ph_mem->drawState.color);
}

int print(std::string str, int x, int y, uint8_t c) {
	_ph_graphics->color(c);

	_ph_mem->drawState.text_x = x;
	_ph_mem->drawState.text_y = y;

	uint8_t effectiveC = _ph_graphics->getDrawPalMappedColor(c);

	//font sprite sheet has text as color 7, with 0 as transparent. We need to override
	//these values and restore them after
	uint8_t prevDrawPal[16];
	for(uint8_t i = 0; i < 16; i++) {
		prevDrawPal[i] = _ph_mem->drawState.drawPaletteMap[i];
		_ph_mem->drawState.drawPaletteMap[i] = i;
	}

	_ph_mem->drawState.drawPaletteMap[7] = effectiveC;
	_ph_mem->drawState.drawPaletteMap[0] = 16; //transparent


	for (size_t n = 0; n < str.length(); n++) {
		uint8_t ch = str[n];
		if (ch == 1) { // "\*{p0}" repeat the next character p0 times
			uint8_t timesChr = str[++n];
			int times = p0CharToNum(timesChr);

			ch = str[++n];
			for(int i = 0; i < times; i++) {
				x = _ph_graphics->drawCharacter(ch, x, y);
			}
		}
		else if (ch == 2) { // "\#{p0}" draw text on a solid background color
			uint8_t bgColChar = str[++n];
			uint8_t bgCol = p0CharToNum(bgColChar);

			_ph_mem->drawState.drawPaletteMap[0] = bgCol;
		}
		else if (ch == 3) { // "\-{p0}" move text cursor horizontally by 16-p0 pixels
			uint8_t pixelCountChar = str[++n];
			int deltaX = p0CharToNum(pixelCountChar) - 16;

			x += deltaX;
			//update memory cursor state?
		}
		else if (ch == 4) { // "\|{p0}" move text cursor vertically by 16-p0 pixels
			uint8_t pixelCountChar = str[++n];
			int deltaY = p0CharToNum(pixelCountChar) - 16;

			y += deltaY;
			//update memory cursor state?
		}
		else if (ch == 5) { // "\+{p0}" move text cursor vertically by 16-p0 pixels
			uint8_t pixelCountXChar = str[++n];
			uint8_t pixelCountYChar = str[++n];
			int deltaX = p0CharToNum(pixelCountXChar) - 16;
			int deltaY = p0CharToNum(pixelCountYChar) - 16;

			x += deltaX;
			y += deltaY;
			//update memory cursor state?
		}
		else if (ch == 6) { // "\^" special command
			uint8_t commandChar = str[++n];
			if (commandChar > 48 && commandChar < 58){
				//pause for x num frames
                int frameCount = pow2(p0CharToNum(commandChar) - 1);
                while (frameCount > 0){
                    _ph_vm->vm_flip();

                    frameCount--;
                }
			}

		}
		else if (ch == 12) { //"\f{p0}" draw text with this foreground color
			uint8_t fgColChar = str[++n];
			uint8_t fgCol = p0CharToNum(fgColChar);
			_ph_graphics->color(fgCol);
			_ph_mem->drawState.drawPaletteMap[7] = _ph_graphics->getDrawPalMappedColor(fgCol);
		}
		else if (ch == '\n') {
			x = _ph_mem->drawState.text_x;
			y += 6;
		}
		else if (ch == '\t') {
			while (x % 16 > 0) {
				x++;
			}
		}
		else if (ch == '\b') {
			x -= 4;
		}
		else if (ch == '\r') {
			x = _ph_mem->drawState.text_x;
		}
		else if (ch >= 0x10) {
			x = _ph_graphics->drawCharacter(ch, x, y);
		}
	}

	for(int i = 0; i < 16; i++) {
		_ph_mem->drawState.drawPaletteMap[i] = prevDrawPal[i];
	}

	//todo: auto scrolling
	_ph_mem->drawState.text_y += 6;

	return x;
}
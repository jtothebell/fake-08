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
    int homeX = x;
    int homeY = y;
    int tabStopWidth = 4;
    int charWidth = 4;
    int charHeight = 6;
    int lineHeight = 6;

    uint8_t printMode = _ph_mem->hwState.printAttributes;

    if ((printMode & 0x1) == 0) {
        printMode = 0;
    }

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

    int framesBetweenChars = 0;
    int framesToPause = 0;
    int rhsWrap = -1;


	for (size_t n = 0; n < str.length(); n++) {
		uint8_t ch = str[n];
        framesToPause = framesBetweenChars;
		if (ch == 1) { // "\*{p0}" repeat the next character p0 times
			uint8_t timesChr = str[++n];
			int times = p0CharToNum(timesChr);

			ch = str[++n];
			for(int i = 0; i < times; i++) {
				x += charWidth +  _ph_graphics->drawCharacter(ch, x, y, printMode);
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
            // \^1 through \^9 skips a number of frames before continuing printing
			if (commandChar > 48 && commandChar < 58){
				//pause for x num frames
                int frameCount = pow2(p0CharToNum(commandChar) - 1);
                while (frameCount > 0){
                    _ph_vm->vm_flip();

                    frameCount--;
                }
			}
            // \^d P0 causes subsequent printing to pause P0 frames between each character,
            else if (commandChar == 'd') {
                uint8_t framesChar = str[++n];
                framesBetweenChars = p0CharToNum(framesChar);
            }
            else if (commandChar == 'c'){
                uint8_t colChar = str[++n];
                uint8_t col = p0CharToNum(colChar);

                _ph_graphics->cls(col);
            }
            else if (commandChar == 'g'){
                x = homeX;
                y = homeY;
            }
            else if (commandChar == 'h'){
                homeX = x;
                homeY = y;
            }
            else if (commandChar == 'j'){
                uint8_t xChar = str[++n];
                uint8_t yChar = str[++n];
                x = p0CharToNum(xChar) * 4;
                y = p0CharToNum(yChar) * 4;
            }
            else if (commandChar == 'r'){
                uint8_t rhsWrapChar = str[++n];
                rhsWrap = p0CharToNum(rhsWrapChar) * 4;
            }
            else if (commandChar == 's'){
                uint8_t tswChar = str[++n];
                tabStopWidth = p0CharToNum(tswChar);
            }
            else if (commandChar == 'x'){
                uint8_t charWidthChar = str[++n];
                charWidth = p0CharToNum(charWidthChar);
            }
            else if (commandChar == 'y'){
                //this behaves in a way I wouldn't expect in pico 8- it seems to apply to the next print statment
                //but not if there is a \n in the string. Not sure if this is a bug or not
                uint8_t charHeightChar = str[++n];
                charHeight = p0CharToNum(charHeightChar);
                lineHeight = charHeight > lineHeight ? charHeight : lineHeight;
            }
            else if (commandChar == 'w'){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_WIDE;
                charWidth = 8;
            }
            else if (commandChar == 't'){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_TALL;
                charHeight = 12;
                lineHeight = charHeight > lineHeight ? charHeight : lineHeight;
            }
            else if (commandChar == '='){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_STRIPEY;
            }
            else if (commandChar == 'p'){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_WIDE;
                printMode |= PRINT_MODE_TALL;
                printMode |= PRINT_MODE_STRIPEY;
                charWidth = 8;
                charHeight = 12;
            }
            else if (commandChar == 'i'){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_INVERTED;
            }
            else if (commandChar == 'b'){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_PADDING;
            }
            else if (commandChar == '#'){
                printMode |= PRINT_MODE_ON;
                printMode |= PRINT_MODE_SOLID_BG;
            }
            else if (commandChar == '-'){
                uint8_t turnOffModeChar = str[++n];
                if (printMode) {
                    if (turnOffModeChar == 'w') {
                        printMode &= ~(PRINT_MODE_WIDE);
                        charWidth = 4;
                    }
                    else if (turnOffModeChar == 't') {
                        printMode &= ~(PRINT_MODE_TALL);
                        charHeight = 6;
                        lineHeight = charHeight > lineHeight ? charHeight : lineHeight;
                    }
                    else if (turnOffModeChar == '=') {
                        printMode &= ~(PRINT_MODE_STRIPEY);
                    }
                    else if (turnOffModeChar == 'p') {
                        printMode &= ~(PRINT_MODE_WIDE);
                        printMode &= ~(PRINT_MODE_TALL);
                        printMode &= ~(PRINT_MODE_STRIPEY);
                        charWidth = 4;
                        charHeight = 6;
                        lineHeight = charHeight > lineHeight ? charHeight : lineHeight;
                    }
                    else if (turnOffModeChar == 'i') {
                        printMode &= ~(PRINT_MODE_INVERTED);
                    }
                    else if (turnOffModeChar == 'b') {
                        printMode &= ~(PRINT_MODE_PADDING);
                    }
                    else if (turnOffModeChar == 'b') {
                        printMode &= ~(PRINT_MODE_SOLID_BG);
                    }
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
			y += lineHeight;
		}
		else if (ch == '\t') {
			while (x % (tabStopWidth*4) > 0) {
				x++;
			}
		}
		else if (ch == '\b') {
			x -= charWidth;
		}
		else if (ch == '\r') {
			x = _ph_mem->drawState.text_x;
		}
		else if (ch >= 0x10) {
			x += charWidth + _ph_graphics->drawCharacter(ch, x, y, printMode);
            while (framesToPause > 0){
                _ph_vm->vm_flip();

                framesToPause--;
            }
		}

        //soft wrap if enabled
        if (rhsWrap > 0 && x >= rhsWrap) {
            x = _ph_mem->drawState.text_x;
			y += lineHeight;
        }
	}

	for(int i = 0; i < 16; i++) {
		_ph_mem->drawState.drawPaletteMap[i] = prevDrawPal[i];
	}

	//todo: auto scrolling
	_ph_mem->drawState.text_y += lineHeight;

	return x;
}
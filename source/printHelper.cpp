#include <string>

#include "printHelper.h"
#include "nibblehelpers.h"

#include <string>
#include "graphics.h"
#include "vm.h"
#include "Audio.h"

PicoRam* _ph_mem;
Graphics* _ph_graphics;
Vm* _ph_vm;
Audio* _ph_audio;

void hexStrToBytes(std::string hex, uint8_t byteBuff[]) {
  char buff[3];
  buff[2] = 0;
  for (unsigned int i = 0; i < hex.length(); i += 2) {
    buff[0] = hex[i];
    buff[1] = hex[i+1];
    uint8_t byte = (uint8_t) strtol(buff, NULL, 16);
    byteBuff[i/2] = byte;
  }
}

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
    //todo: default is not 0,0?
    int x = _ph_mem->drawState.text_x;
    int y = _ph_mem->drawState.text_y;
    if (y >= 127) {
        //Memcy screen to itself offset by how many lines needed (y + line height)
        int lineHeight = 6; // possibly need to check if bigger font?
        int linesToCopy = 127 - lineHeight; 
        int startY = y - linesToCopy; 

        memmove(
            &_ph_mem->screenBuffer, 
            &_ph_mem->screenBuffer[COMBINED_IDX(0, startY)],
            linesToCopy * 64);
        
        //set the rest of the screen buffer to 0 (black)
        memset(
            &_ph_mem->screenBuffer[COMBINED_IDX(0, linesToCopy)],
            0,
            startY * 64);

        y = _ph_mem->drawState.text_y = 127 - lineHeight;
        //Memcpy buffy back to screen

    }
	int result = print(str, x, y);

    if (_ph_mem->drawState.text_y >= 127) {
        int lineHeight = 5; // possibly need to check if bigger font?
        int linesToCopy = 127 - lineHeight; 
        int startY = _ph_mem->drawState.text_y - linesToCopy; 
        memmove(
            &_ph_mem->screenBuffer, 
            &_ph_mem->screenBuffer[COMBINED_IDX(0, startY)],
            linesToCopy * 64);
        
        //set the rest of the screen buffer to 0 (black)
        memset(
            &_ph_mem->screenBuffer[COMBINED_IDX(0, linesToCopy)],
            0,
            startY * 64);

        y = _ph_mem->drawState.text_y = 127 - lineHeight;
    }

	return result;
}

int print(std::string str, int x, int y) {
	return print(str, x, y, _ph_mem->drawState.color);
}

int print(std::string str, int x, int y, uint8_t c) {
	_ph_graphics->color(c);

	_ph_mem->drawState.text_x = x;
	_ph_mem->drawState.text_y = y;
    int length = str.length();
    int homeX = x;
    int homeY = y;
    int prevX = x;
    int prevY = y;
    int tabStopWidth = 4;
    int charWidth = 4;
    int charHeight = 6;
    int lineHeight = 0;
    int forceCharWidth = -1;
    int forceCharHeight = -1;
    uint8_t bgColor = 0;
    uint8_t fgColor = _ph_mem->drawState.color;
    uint8_t charBytes[8];
    uint8_t audioStrBytes[32];
    bool cancelWrap = false;

    uint8_t printMode = _ph_mem->hwState.printAttributes;

    if ((printMode & 0x1) == 0) {
        printMode = 0;
    }

    if ((printMode & PRINT_MODE_CUSTOM_FONT)) {
        charWidth = _ph_mem->data[0x5600];
        charHeight = _ph_mem->data[0x5602];
    }

    uint8_t* drawPal = _ph_mem->drawState.drawPaletteMap;

    int framesBetweenChars = 0;
    int framesToPause = 0;
    int rhsWrap = -1;


	for (size_t n = 0; n < length; n++) {
		uint8_t ch = str[n];
        framesToPause = framesBetweenChars;
        if (ch == 0) { // null stop printing
            _ph_mem->drawState.text_x=x;
            _ph_mem->drawState.text_y=y;

            return x;
        }
        else if (ch == 1) { // "\*{p0}" repeat the next character p0 times
			uint8_t timesChr = str[++n];
			int times = p0CharToNum(timesChr);

			ch = str[++n];
			for(int i = 0; i < times; i++) {
                //TODO: combine with other draw character call - fix missing bg? lineheight?
				x += charWidth +  _ph_graphics->drawCharacter(
                    ch,
                    x,
                    y,
                    drawPal[fgColor & 0x0f],
                    drawPal[bgColor & 0x0f],
                    printMode,
                    forceCharWidth,
                    forceCharHeight);
			}
		}
		else if (ch == 2) { // "\#{p0}" draw text on a solid background color
			uint8_t bgColChar = str[++n];
			bgColor = p0CharToNum(bgColChar);
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
                forceCharWidth = p0CharToNum(charWidthChar);
                charWidth = forceCharWidth;
            }
            else if (commandChar == 'y'){
                //this behaves in a way I wouldn't expect in pico 8- it seems to apply to the next print statment
                //but not if there is a \n in the string. Not sure if this is a bug or not
                uint8_t charHeightChar = str[++n];
                forceCharHeight = p0CharToNum(charHeightChar);
                charHeight = forceCharHeight;
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
            else if (commandChar == ':' || commandChar == '.'){
                charHeight = forceCharHeight > 0 ? forceCharHeight : 8;
                if (commandChar == ':') {
                    std::string hexStr = str.substr(n+1, 16);
                    n+=16;
                    hexStrToBytes(hexStr, charBytes);
                }
                else {
                    std::string binStr = str.substr(n+1, 8);
                    n+=8;
                    for(size_t i = 0; i < 8; i++) {
                        charBytes[i] = binStr[i];
                    }
                }

                auto values = _ph_graphics->drawCharacterFromBytes(
                    charBytes,
                    x,
                    y,
                    drawPal[fgColor & 0x0f],
                    drawPal[bgColor & 0x0f],
                    printMode,
                    8,
                    charHeight);

                x += 8 + get<0>(values);
                charHeight = charHeight + get<1>(values);
                lineHeight = charHeight > lineHeight ? charHeight : lineHeight;
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
            else if (commandChar == '!'){
                //no wraping after this print call (may be pico 8 bug?)
                cancelWrap = true;

                std::string addrHexStr = str.substr(n+1, 4);
                n+=4;
                int addr = (int)strtol(addrHexStr.c_str(), NULL, 16);

                int size = length - n;
                std::string binStr = str.substr(n + 1, size);
                n+=size;

                for(size_t i = 0; i < size; i++) {
                    _ph_mem->data[addr + i] = binStr[i];
                }
            }
            else if (commandChar == '@'){
                std::string addrHexStr = str.substr(n+1, 4);
                n+=4;
                int addr = (int)strtol(addrHexStr.c_str(), NULL, 16);

                std::string sizeHexStr = str.substr(n+1, 4);
                n+=4;

                int size = (int)strtol(sizeHexStr.c_str(), NULL, 16);

                std::string binStr = str.substr(n + 1, size);
                n+= size;

                for(size_t i = 0; i < size; i++) {
                    _ph_mem->data[addr + i] = binStr[i];
                }
            }
		}
		else if (ch == 7) { // "\a" audio command
            uint8_t nextChar = str[++n];
            int audioStrIdx = 0;
            while (nextChar != 32 && n < length) {
                audioStrBytes[audioStrIdx++] = nextChar;
                nextChar = str[++n];
            }
            //todo: generate audio stuff and play
        }
        else if (ch == 11) { // "\v" decorate previous character
            uint8_t offsetChar = str[++n];
			uint8_t toPrint = str[++n];
            int offset = p0CharToNum(offsetChar);

            int xOffset = (offset%4)-2;
            int yOffset = (offset/4)-8;

            _ph_graphics->drawCharacter(
                toPrint,
                prevX + xOffset,
                prevY + yOffset,
                drawPal[fgColor & 0x0f],
                drawPal[bgColor & 0x0f],
                printMode,
                forceCharWidth,
                forceCharHeight);
        }
        else if (ch == 12) { //"\f{p0}" draw text with this foreground color
			uint8_t fgColChar = str[++n];
			fgColor = p0CharToNum(fgColChar);
			_ph_graphics->color(fgColor);
		}
        else if (ch == 14) { //"\014" Turn on custom font stored at 0x5600
            printMode |= PRINT_MODE_CUSTOM_FONT;
		}
        else if (ch == 15) { //"\015" Turn off custom font stored at 0x5600
            printMode &= ~(PRINT_MODE_CUSTOM_FONT);
		}
		else if (ch == '\n') {
			x = homeX;
            lineHeight = lineHeight > 0 ? lineHeight : 6;
			y += lineHeight;
            lineHeight = 0;
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
			x = homeX;
		}
		else if (ch >= 0x10) {
            lineHeight = charHeight > lineHeight ? charHeight : lineHeight;
            if (bgColor != 0) {
                uint8_t prevPenColor = _ph_mem->drawState.color;
                _ph_graphics->rectfill(x-1, y-1, x + charWidth-1, y + lineHeight-1, bgColor);
                _ph_mem->drawState.color = prevPenColor;
            }
            prevX = x;
            prevY = y;
			x += charWidth + _ph_graphics->drawCharacter(
                ch,
                x,
                y,
                drawPal[fgColor & 0x0f],
                drawPal[bgColor & 0x0f],
                printMode,
                forceCharWidth,
                forceCharHeight);

            while (framesToPause > 0){
                _ph_vm->vm_flip();

                framesToPause--;
            }
		}

        //soft wrap if enabled
        if (rhsWrap > 0 && x >= rhsWrap) {
            x = homeX;
            lineHeight = lineHeight > 0 ? lineHeight : 6;
			y += lineHeight;
            lineHeight = 0;
        }
	}

    lineHeight = lineHeight > 0 ? lineHeight : cancelWrap ? 0 : 6;
	//todo: auto scrolling
	_ph_mem->drawState.text_y = y + lineHeight;

	return x;
}

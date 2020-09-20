
#include <string>

#include "nibblehelpers.h"

int getCombinedIdx(int x, int y){
    return y * 64 + (x / 2);
}

void setPixelNibble(int x, int y, uint8_t value, uint8_t* targetBuffer) {
    int combinedIdx = getCombinedIdx(x, y);

	uint8_t currentByte = targetBuffer[combinedIdx];
	uint8_t mask = x % 2 == 0 ? 0x0f : 0xf0;
	value = mask == 0xf0 ? value << 4 : value;

	targetBuffer[combinedIdx] = (currentByte & ~mask) | (value & mask);
}

uint8_t getPixelNibble(int x, int y, uint8_t* targetBuffer) {
    int combinedIdx = getCombinedIdx(x, y);

	uint8_t combinedPix = targetBuffer[combinedIdx];

	uint8_t c = x % 2 == 0 
		? combinedPix & 0x0f //just first 4 bits
		: combinedPix >> 4;  //just last 4 bits
		
	return c;
}
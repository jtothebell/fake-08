
#include <string>

#include "hostVmShared.h"
#include "nibblehelpers.h"

int getCombinedIdx(int x, int y){
	//bit shifting might be faster? trying to optimize
    //return y * 64 + (x / 2);
	return (y << 6) + (x >> 1);
}

void setPixelNibble(int x, int y, uint8_t value, uint8_t* targetBuffer) {
    int combinedIdx = getCombinedIdx(x, y);

	uint8_t currentByte = targetBuffer[combinedIdx];
	//get mask for even or odd
	uint8_t mask = (BITMASK(0) & x) == 0 ? 0x0f : 0xf0;
	value = mask == 0xf0 ? value << 4 : value;

	targetBuffer[combinedIdx] = (currentByte & ~mask) | (value & mask);
}

uint8_t getPixelNibble(int x, int y, uint8_t* targetBuffer) {
    int combinedIdx = getCombinedIdx(x, y);

	uint8_t combinedPix = targetBuffer[combinedIdx];

	uint8_t c = (BITMASK(0) & x) == 0 
		? combinedPix & 0x0f //just first 4 bits
		: combinedPix >> 4;  //just last 4 bits
		
	return c;
}
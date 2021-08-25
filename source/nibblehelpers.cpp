
#include <string>

#include "hostVmShared.h"
#include "nibblehelpers.h"


#define COMBINED_IDX(x, y) ((y) << 6) + ((x) >> 1)

int getCombinedIdx(const int x, const int y){
	//bit shifting might be faster? trying to optimize
    //return y * 64 + (x / 2);
	//return (y << 6) + (x >> 1);
	return COMBINED_IDX(x, y);
}
//try look up table to optimize?

void setPixelNibble(const int x, const int y, uint8_t value, uint8_t* targetBuffer) {
    const int combinedIdx = COMBINED_IDX(x, y);

	const uint8_t currentByte = targetBuffer[combinedIdx];
	//get mask for even or odd
	const uint8_t mask = (BITMASK(0) & x) == 0 ? 0x0f : 0xf0;
	value = mask == 0xf0 ? value << 4 : value;

	targetBuffer[combinedIdx] = (currentByte & ~mask) | (value & mask);

	//potentially slightly optimized by inlining - don't have measurements to back it up
	//(BITMASK(0) & x) == 0 
	//	? targetBuffer[(y << 6) + (x >> 1)] = (targetBuffer[(y << 6) + (x >> 1)] & 0xf0) | (value & 0x0f)
	//	: targetBuffer[(y << 6) + (x >> 1)] = (targetBuffer[(y << 6) + (x >> 1)] & 0x0f) | (value << 4 & 0xf0);
}

uint8_t getPixelNibble(const int x, const int y, const uint8_t* targetBuffer) {
	return  (BITMASK(0) & x) == 0 
		? targetBuffer[COMBINED_IDX(x, y)] & 0x0f //just first 4 bits
		: targetBuffer[COMBINED_IDX(x, y)] >> 4;  //just last 4 bits
}
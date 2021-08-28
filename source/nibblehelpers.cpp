
#include <string>

#include "hostVmShared.h"
#include "nibblehelpers.h"



int getCombinedIdx(const int x, const int y){
	//bit shifting might be faster? trying to optimize
    //return y * 64 + (x / 2);
	//return (y << 6) | (x >> 1);
	return COMBINED_IDX(x, y);
}
//try look up table to optimize?

void setPixelNibble(const int x, const int y, uint8_t value, uint8_t* targetBuffer) {
	//potentially slightly optimized by inlining - don't have measurements to back it up
	targetBuffer[COMBINED_IDX(x, y)] = (BITMASK(0) & x)
		? (targetBuffer[COMBINED_IDX(x, y)] & 0x0f) | (value << 4 & 0xf0)
		: (targetBuffer[COMBINED_IDX(x, y)] & 0xf0) | (value & 0x0f);
}

uint8_t getPixelNibble(const int x, const int y, const uint8_t* targetBuffer) {
	return (BITMASK(0) & x) 
		? targetBuffer[COMBINED_IDX(x, y)] >> 4  //just last 4 bits
		: targetBuffer[COMBINED_IDX(x, y)] & 0x0f; //just first 4 bits
}
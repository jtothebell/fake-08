#pragma once

#include <string>

//for 1 byte (8 bit) indexes, 128x64
//should be the equivalent of return y * 64 + (x / 2);
#define COMBINED_IDX(x, y) ((y) << 6) | ((x) >> 1)
//I think this should work if you cast the buffer to a uint32_t* pointer, but not tested
//for 4 byte (32 bit) inexes, 16x8
//should be the equivalent of return y * 16 + (x / 8);
//#define COMBINED_32_BIT_IDX(x, y) ((y) << 3) | ((x) >> 3)
//this may be hlpeful to optimize sprite blitting
//idea: get uint32_t from sprite buffer- should be 8 pixels
//split it up by bit shifting, and write to screen buffer as necessary

int getCombinedIdx(int x, int y);

void setPixelNibble(const int x, const int y, uint8_t value, uint8_t* targetBuffer);

uint8_t getPixelNibble(const int x, const int y, const uint8_t* targetBuffer);

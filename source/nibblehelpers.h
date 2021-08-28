#pragma once

#include <string>

//should be the equivalent of return y * 64 + (x / 2);
#define COMBINED_IDX(x, y) ((y) << 6) | ((x) >> 1)

int getCombinedIdx(int x, int y);

void setPixelNibble(const int x, const int y, uint8_t value, uint8_t* targetBuffer);

uint8_t getPixelNibble(const int x, const int y, const uint8_t* targetBuffer);

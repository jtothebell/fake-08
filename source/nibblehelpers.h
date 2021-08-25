#pragma once

#include <string>

int getCombinedIdx(int x, int y);

void setPixelNibble(const int x, const int y, uint8_t value, uint8_t* targetBuffer);

uint8_t getPixelNibble(const int x, const int y, const uint8_t* targetBuffer);

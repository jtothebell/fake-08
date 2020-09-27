#pragma once

#include <string>

int getCombinedIdx(int x, int y);

void setPixelNibble(int x, int y, uint8_t value, uint8_t* targetBuffer);

uint8_t getPixelNibble(int x, int y, uint8_t* targetBuffer);

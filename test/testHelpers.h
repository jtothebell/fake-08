#pragma once

#include <vector>

#include "../source/hostVmShared.h"
#include "../source/graphics.h"

struct coloredPoint {
    uint8_t x;
    uint8_t y;
    uint8_t c;
};

bool colorsEqual(Color* lhs, Color* rhs);

void debugScreen(Graphics* graphics);

void checkPoints(Graphics* graphics, std::vector<coloredPoint> expectedPoints);

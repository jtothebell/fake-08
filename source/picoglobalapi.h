#pragma once

#include <string>

#include "graphics.h"

//this can probably go away when I'm loading actual carts and just have to expose api to lua
void initGlobalApi(Graphics* graphics);

void cls();
void pset(short x, short y, uint8_t col);
uint8_t pget(short x, short y);
void color(uint8_t c);
void line (short x1, short y1, short x2, short y2, uint8_t col);
void circ(short ox, short oy, short r, uint8_t col);
void circfill(short ox, short oy, short r, uint8_t col);
void rect(short x1, short y1, short x2, short y2, uint8_t col);
void rectfill(short x1, short y1, short x2, short y2, uint8_t col);
short print(std::string str, short x, short y, uint16_t c);
void spr(int n, int x, int y, int w, int h, bool flip_x, bool flip_y);

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "graphics.h"


const Color BgGray = BG_GRAY_COLOR;

const Color PaletteColors[] = {
	COLOR_00,
	COLOR_01,
	COLOR_02,
	COLOR_03,
	COLOR_04,
	COLOR_05,
	COLOR_06,
	COLOR_07,
	COLOR_08,
	COLOR_09,
	COLOR_10,
	COLOR_11,
	COLOR_12,
	COLOR_13,
	COLOR_14,
	COLOR_15
};


uint8_t _pico8_fb[128*128]; 

static GraphicsState _graphicsState;

SpriteSheet _fontSpriteSheet;

void copy_data_to_sprites(SpriteSheet *sprites, std::string data) {
	uint16_t i = 0;

	for (size_t n = 0; n < data.length(); n++) {
		char buf[3] = {0};

		if (data[n] > ' ') {
			buf[0] = data[n++];
			buf[1] = data[n];
			uint8_t val = (uint8_t)strtol(buf, NULL, 16);

			sprites->sprite_data[i++] = val >> 4;
			sprites->sprite_data[i++] = val & 0x0f;
		}
	}
}


//call initialize to make sure defaults are correct
void initPico8Graphics(std::string fontdata) {

	copy_data_to_sprites(&_fontSpriteSheet, fontdata);
}

//based on tac08 implementation of blitter()
void copySpriteToScreen(
	uint8_t spritebuffer[],
	short scr_x,
	short scr_y,
	short spr_x,
	short spr_y,
	short spr_w,
	short spr_h,
	bool flip_x = false,
	bool flip_y = false) 
{

	//note: no clipping yet
	short scr_w = spr_w;
	short scr_h = spr_h;
	
	short dy = 1;

	for (short y = 0; y < scr_h; y++) {
		uint8_t* spr = spritebuffer + ((spr_y + y * dy) & 0x7f) * 128;

		for (short x = 0; x < scr_w; x++) {
			uint8_t c = spr[(spr_x + x) & 0x7f];
			if (c != 0) { //if not transparent. Come back later to add palt() support by checking tranparency palette
				pset(scr_x + x, scr_y + y, c); //set color on framebuffer. Come back later and add pal() by translating color
			}
		}
	}
}


//start helper methods
void swap(short *x, short *y) {
   short temp;
   temp = *x;
   *x = *y;
   *y = temp;
  
   return;
}

void sortPointsLtoR(short *x1, short *y1, short *x2, short *y2){
	if (*x1 > *x2) {
		swap(x1, x2);
		swap(y1, y2);
	}
}

void sortCoordsForRect(short *x1, short *y1, short *x2, short *y2){
	if (*x1 > *x2) {
		swap(x1, x2);
	}

	if (*y1 > *y2) {
		swap(y1, y2);
	}
}

bool isOnScreen(short *x, short* y) {
	return *x >= 0 && *x < 127 && *y >= 0 && *y < 127;
}
//end helper methods

void cls() {
	memset(_pico8_fb, _graphicsState.bgColor, sizeof(_pico8_fb));
}

void pset(short x, short y, uint8_t col){
	if (isOnScreen(&x, &y)){
		_pico8_fb[(x * 128) + y] = col;
	}
}

uint8_t pget(short x, short y){
	if (isOnScreen(&x, &y)){
		return _pico8_fb[(x * 128) + y];
	}

	return 0;
}


void color(uint8_t col){
	_graphicsState.color = col;
}

void line (short x1, short y1, short x2, short y2, uint8_t col) {
	sortPointsLtoR(&x1, &y1, &x2, &y2);

	float run = x2 - x1;
	float rise = y2 - y1;

	//vertical line
	if (run == 0) {
		if (y1 > y2) {
			swap(&y1, &y2);
		}

		for (short y = y1; y <= y2; y++){
			pset(x1, y, col);
		}
	}
	else {
		float slope = rise / run;

		for (short x = x1; x <= x2; x++){
			short y = y1 + (short)ceil((float)x * slope);
			pset(x, y, col);
		}
	}
}

void circ(short ox, short oy, short r, uint8_t col){
	short x = r;
	short y = 0;
	short decisionOver2 = 1-x;

	while (y <= x) {
		pset(ox + x, oy + y, col);
		pset(ox + y, oy + x, col);
		pset(ox - x, oy + y, col);
		pset(ox - y, oy + x, col);

		pset(ox - x, oy - y, col);
		pset(ox - y, oy - x, col);
		pset(ox + x, oy - y, col);
		pset(ox + y, oy - x, col);

		y += 1;
		if (decisionOver2 < 0) {
			decisionOver2=decisionOver2+2*y+1;
		}
		else {
			x = x-1;
			decisionOver2 = decisionOver2 + 2 * (y - x) + 1;
		}
	}

}

void circfill(short ox, short oy, short r, uint8_t col){
	if (r == 0) {
		pset(ox, oy, col);
	}
	else if (r == 1) {
		pset(ox, oy - 1, col);
		line(ox-1, oy, ox+1, oy, col);
		pset(ox, oy + 1, col);
	}
	else if (r > 0) {
		short x = -r, y = 0, err = 2 - 2 * r;
		do {
			line(ox - x, oy + y, ox + x, oy + y, col);
			line(ox - x, oy - y, ox + x, oy - y, col);
			r = err;
			if (r > x)
				err += ++x * 2 + 1;
			if (r <= y)
				err += ++y * 2 + 1;
		} while (x < 0);
	}
	
}

void rect(short x1, short y1, short x2, short y2, uint8_t col) {
	sortCoordsForRect(&x1, &y1, &x2, &y2);

	for (short i = x1; i <= x2; i++) {
		for (short j = y1; j <= y2; j++) {
			if ((i == x1 || i == x2 || j == y1 || j == y2) ) {
				pset(i, j, col);
			}
		}
	}

}

void rectfill(short x1, short y1, short x2, short y2, uint8_t col) {
	sortCoordsForRect(&x1, &y1, &x2, &y2);

	for (short i = x1; i <= x2; i++) {
		for (short j = y1; j <= y2; j++) {
			pset(i, j, col);
		}
	}
}

//tac08
short print(std::string str, short x, short y, uint16_t c) {
	_graphicsState.text_x = x;

	for (size_t n = 0; n < str.length(); n++) {
		uint8_t ch = str[n];
		if (ch >= 0x10 && ch < 0x80) {
			short index = ch - 0x10;
			copySpriteToScreen(_fontSpriteSheet.sprite_data, x, y, (index % 16) * 8, (index / 16) * 8, 4, 5);
			x += 4;
		} else if (ch >= 0x80) {
			short index = ch - 0x80;
			copySpriteToScreen(_fontSpriteSheet.sprite_data, x, y, (index % 16) * 8, (index / 16) * 8 + 56, 8, 5);
			x += 8;
		} else if (ch == '\n') {
			x = _graphicsState.text_x;
			y += 6;
		}
	}

	_graphicsState.text_x = 0;
	_graphicsState.text_y = y + 6;

	return x;
}

void flipBuffer(uint8_t* fb) {
	short x, y;
    for(x = 0; x < 400; x++) {
    	for(y = 0; y < 240; y++) {
			if (x < 128 && y < 128) {
				uint8_t c = _pico8_fb[x*128 + y];
				Color col = PaletteColors[c];

				fb[((x*240)+ (239 - y))*3+0] = col.Blue;
				fb[((x*240)+ (239 - y))*3+1] = col.Green;
				fb[((x*240)+ (239 - y))*3+2] = col.Red;
			}
    	}
    }
}


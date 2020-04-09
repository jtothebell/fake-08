#include <math.h>
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics.h"


/*
typedef struct Point{
	char X;
	char Y;
};

typedef struct Dimensions{
	char Width;
	char Height;
};
*/

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


uint16_t _pico8_fb[128*128]; 

GraphicsState _graphicsState;

//call initialize to make sure defaults are correct
void initPico8Graphics() {
	_graphicsState.bgColor = 0;
	_graphicsState.color = 7;
}

//start helper methods
void swap(short *x, short *y) {
   short temp;
   temp = *x;
   *x = *y;
   *y = temp;
  
   return;
}

bool isOnScreen(short *x, short* y) {
	return *x >= 0 && *x < 127 && *y >= 0 && *y < 127;
}
//end helper methods

void cls() {
	memset(_pico8_fb, _graphicsState.bgColor, sizeof(_pico8_fb));
}

void pset(short x, short y, uint16_t col){
	if (isOnScreen(&x, &y)){
		_pico8_fb[(x * 128) + y] = col;
	}
}

uint16_t pget(short x, short y){
	if (isOnScreen(&x, &y)){
		return _pico8_fb[(x * 128) + y];
	}

	return 0;
}

void color(uint16_t col){
	_graphicsState.color = col;
}

void line (short x1, short y1, short x2, short y2, uint16_t col) {
	if (x1 > x2) {
		swap(&x1, &x2);
		swap(&y1, &y2);
	}

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

void rect(short x1, short y1, short x2, short y2, uint16_t col) {
	if (x1 > x2) {
		swap(&x1, &x2);
		swap(&y1, &y2);
	}

	for (short i = x1; i <= x2; i++) {
		for (short j = y1; j <= y2; j++) {
			if ((i == x1 || i == x2 || j == y1 || j == y2) ) {
				pset(i, j, col);
			}
		}
	}

}

void rectfill(short x1, short y1, short x2, short y2, uint16_t col) {
	if (x1 > x2) {
		swap(&x1, &x2);
		swap(&y1, &y2);
	}

	for (short i = x1; i <= x2; i++) {
		for (short j = y1; j <= y2; j++) {
			pset(i, j, col);
		}
	}
}

void flipBuffer(u8* fb) {
	int x, y;
    for(x = 0; x < 400; x++) {
    	for(y = 0; y < 240; y++) {
			if (x < 128 && y < 128) {
				uint16_t c = _pico8_fb[x*128 + y];
				Color col = PaletteColors[c];

				fb[((x*240)+ (239 - y))*3+0] = col.Blue;
				fb[((x*240)+ (239 - y))*3+1] = col.Green;
				fb[((x*240)+ (239 - y))*3+2] = col.Red;
			}
    	}
    }

}



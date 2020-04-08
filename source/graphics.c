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

void cls() {
	memset(_pico8_fb, 0, sizeof(_pico8_fb));
}

void rect(char x, char y, char x1, char y1, uint16_t col) {
	/*
	char w = x1 - x;
	char h = y1 - y;

	char currentX = 0;
	char currentY = 0;
	*/


}

void rectfill(char x, char y, char x1, char y1, uint16_t col) {
	char holder;
	if (x1 < x) {
		holder = x;
		x = x1;
		x1 = holder;
	}
	if (y1 < y) {
		holder = y;
		y = y1;
		y1 = holder;
	}

	int width = x1 - x;
	int height = y1 - y;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (x < 128 && y < 128) {
				_pico8_fb[((x + i) * 128) + (y + j)] = col;
			}
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

				fb[((x*240)+ (239 - y))*3+0] = col.Red;
				fb[((x*240)+ (239 - y))*3+1] = col.Green;
				fb[((x*240)+ (239 - y))*3+2] = col.Blue;
			}
    	}
    }

}



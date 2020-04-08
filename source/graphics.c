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

void swap(short *x, short *y) {
   short temp;
   temp = *x;
   *x = *y;
   *y = temp;
  
   return;
}

void sortcoords(short *expectedLower, short *expectedHigher){
	if (*expectedHigher < *expectedLower) {
		swap(expectedLower, expectedHigher);
	}
}

void cls() {
	memset(_pico8_fb, 0, sizeof(_pico8_fb));
}

void rect(short x, short y, short x1, short y1, uint16_t col) {
	/*
	char w = x1 - x;
	char h = y1 - y;

	char currentX = 0;
	char currentY = 0;
	*/


}

void rectfill(short x1, short y1, short x2, short y2, uint16_t col) {
	sortcoords(&x1, &x2);
	sortcoords(&y1, &y2);

	for (int i = x1; i <= x2; i++) {
		for (int j = y1; j <= y2; j++) {
			if (i >= 0 && i < 127 && j >= 0 && j < 127) {
				_pico8_fb[(i * 128) + j] = col;
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

				fb[((x*240)+ (239 - y))*3+0] = col.Blue;
				fb[((x*240)+ (239 - y))*3+1] = col.Green;
				fb[((x*240)+ (239 - y))*3+2] = col.Red;
			}
    	}
    }

}



#include <math.h>
#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

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

void clearscreen(u8* fb, int time) {
    int x, y;
    for(x = 0; x < 400; x++) {
    	for(y = 0; y < 240; y++) {
			if (x < 128 && y < 128) {
				/*
				float b = (((sin(y*sin(time/40.0)*M_PI*10.0/400.0)+1)/4) + ((sin(x*sin(time/77.0)*M_PI*13.0/400.0)+1)/4));
				float g = (((sin(y*sin(time/53.0)*M_PI*23.0/400.0)+1)/4) + ((sin(x*sin(time/97.0)*M_PI*17.0/400.0)+1)/4));
				float r = (((sin(y*sin(time/27.0)*M_PI*17.0/400.0)+1)/4) + ((sin(x*sin(time/27.0)*M_PI*31.0/400.0)+1)/4));
				fb[((x*240)+y)*3+0] = (int)(b*255);
				fb[((x*240)+y)*3+1] = (int)(g*255);
				fb[((x*240)+y)*3+2] = (int)(r*255);
				*/
				fb[((x*240)+y)*3+0] = 0;
				fb[((x*240)+y)*3+1] = 0;
				fb[((x*240)+y)*3+2] = 0;
			}
			else {
				//gray
				fb[((x*240)+y)*3+0] = BgGray.Red;
				fb[((x*240)+y)*3+1] = BgGray.Green;
				fb[((x*240)+y)*3+2] = BgGray.Blue;
			}
    	}
    }
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

				fb[((x*240)+y)*3+0] = col.Red;
				fb[((x*240)+y)*3+1] = col.Green;
				fb[((x*240)+y)*3+2] = col.Blue;
			}
    	}
    }

}



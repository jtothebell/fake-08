#pragma once

#include <string>

#define COLOR_00 {  0,   0,   0, 255}
#define COLOR_01 { 29,  43,  83, 255}
#define COLOR_02 {126,  37,  83, 255}
#define COLOR_03 {  0, 135,  81, 255}
#define COLOR_04 {171,  82,  54, 255}
#define COLOR_05 { 95,  87,  79, 255}
#define COLOR_06 {194, 195, 199, 255}
#define COLOR_07 {255, 241, 232, 255}
#define COLOR_08 {255,   0,  77, 255}
#define COLOR_09 {255, 163,   0, 255}
#define COLOR_10 {255, 240,  36, 255}
#define COLOR_11 {  0, 231,  86, 255}
#define COLOR_12 { 41, 173, 255, 255}
#define COLOR_13 {131, 118, 156, 255}
#define COLOR_14 {255, 119, 168, 255}
#define COLOR_15 {255, 204, 170, 255}

#define BG_GRAY_COLOR {128, 128, 128, 255}


struct Color {
	char Red;
	char Green;
	char Blue;
	char Alpha;
};

struct GraphicsState{
    uint8_t color = 7;
    uint8_t bgColor = 0;

    short text_x = 0;
	short text_y = 0;
};

class Graphics {
	uint8_t _pico8_fb[128*128]; 
	GraphicsState* _graphicsState;
	uint8_t fontSpriteData[128 * 128];

	void copySpriteToScreen(
		uint8_t spritebuffer[],
		short scr_x,
		short scr_y,
		short spr_x,
		short spr_y,
		short spr_w,
		short spr_h,
		bool flip_x,
		bool flip_y);

	void swap(short *x, short *y);

	void sortPointsLtoR(short *x1, short *y1, short *x2, short *y2);

	void sortCoordsForRect(short *x1, short *y1, short *x2, short *y2);

	bool isOnScreen(short *x, short* y);

	public:
	Graphics(std::string fontdata);
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

	void flipBuffer(uint8_t* fb);

};


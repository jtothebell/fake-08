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

class Graphics {
	uint8_t _pico8_fb[128*128];
	uint8_t fontSpriteData[128 * 64];

	uint8_t spriteSheetData[128 * 64];
	uint8_t mapData[128 * 64];
	uint8_t spriteFlags[256];

	uint8_t _gfxState_color;
    uint8_t _gfxState_bgColor;

    short _gfxState_text_x;
	short _gfxState_text_y;

	short _gfxState_camera_x;
	short _gfxState_camera_y;

	//not actually part of graphics state memory?
	short _gfxState_line_x;
	short _gfxState_line_y;
	bool _gfxState_line_valid;


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

	void copyStretchSpriteToScreen(
		uint8_t spritebuffer[],
		int spr_x,
		int spr_y,
		int spr_w,
		int spr_h,
		int scr_x,
		int scr_y,
		int scr_w,
		int scr_h,
		bool flip_x,
		bool flip_y);

	void swap(short *x, short *y);

	void sortPointsLtoR(short *x1, short *y1, short *x2, short *y2);

	void sortCoordsForRect(short *x1, short *y1, short *x2, short *y2);

	bool isOnScreen(short x, short y);

	void _private_pset(short x, short y, uint8_t col);

	public:
	Graphics(std::string fontdata);

	void setSpriteSheet(std::string spriteSheetString);
	void setSpriteFlags(std::string spriteFlagsString);

	void cls();
	void cls(uint8_t color);

	void pset(short x, short y);
	void pset(short x, short y, uint8_t col);
	uint8_t pget(short x, short y);

	void color(uint8_t c);

	void line ();
	void line (uint8_t col);
	void line (short x1, short y1);
	void line (short x1, short y1, uint8_t col);
	void line (short x1, short y1, short x2, short y2);
	void line (short x1, short y1, short x2, short y2, uint8_t col);

	void circ(short ox, short oy);
	void circ(short ox, short oy, short r);
	void circ(short ox, short oy, short r, uint8_t col);
	void circfill(short ox, short oy);
	void circfill(short ox, short oy, short r);
	void circfill(short ox, short oy, short r, uint8_t col);

	void rect(short x1, short y1, short x2, short y2);
	void rect(short x1, short y1, short x2, short y2, uint8_t col);
	void rectfill(short x1, short y1, short x2, short y2);
	void rectfill(short x1, short y1, short x2, short y2, uint8_t col);

	short print(std::string str);
	short print(std::string str, short x, short y);
	short print(std::string str, short x, short y, uint16_t c);

	void spr(
		short n,
		short x,
		short y,
		double w = 1.0,
		double h = 1.0,
		bool flip_x = false,
		bool flip_y = false);

	void sspr(
        short sx,
        short sy,
        short sw,
        short sh,
        short dx,
        short dy,
        short dw,
        short dh,
        bool flip_x = false,
        bool flip_y = false);

	bool fget(uint8_t n, uint8_t f);
	uint8_t fget(uint8_t n);
	void fset(uint8_t n, uint8_t f, bool v);
	void fset(uint8_t n, uint8_t v);

	uint8_t sget(uint8_t x, uint8_t y);
	void sset(uint8_t x, uint8_t y, uint8_t c);

	void camera();
	void camera(short x, short y);


	void flipBuffer(uint8_t* fb);

};


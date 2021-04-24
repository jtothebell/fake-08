#pragma once

#include <string>
#include <tuple>
#include "hostVmShared.h"
#include "PicoRam.h"
#include <fix32.h>
using namespace z8;

#define COLOR_00 {  2,   4,   8, 255}
#define COLOR_01 { 29,  43,  83, 255}
#define COLOR_02 {126,  37,  83, 255}
#define COLOR_03 {  0, 135,  81, 255}
#define COLOR_04 {171,  82,  54, 255}
#define COLOR_05 { 95,  87,  79, 255}
#define COLOR_06 {194, 195, 199, 255}
#define COLOR_07 {255, 241, 232, 255}
#define COLOR_08 {255,   0,  77, 255}
#define COLOR_09 {255, 163,   0, 255}
#define COLOR_10 {255, 236,  39, 255}
#define COLOR_11 {  0, 228,  54, 255}
#define COLOR_12 { 41, 173, 255, 255}
#define COLOR_13 {131, 118, 156, 255}
#define COLOR_14 {255, 119, 168, 255}
#define COLOR_15 {255, 204, 170, 255}

//alt palette
#define COLOR_128 { 41,  24,  20, 255}
#define COLOR_129 { 17,  29,  53, 255}
#define COLOR_130 { 66,  33,  54, 255}
#define COLOR_131 { 18,  83,  89, 255}
#define COLOR_132 {116,  47,  41, 255}
#define COLOR_133 { 73,  51,  59, 255}
#define COLOR_134 {162, 136, 121, 255}
#define COLOR_135 {243, 239, 125, 255}
#define COLOR_136 {190,  18,  80, 255}
#define COLOR_137 {255, 108,  36, 255}
#define COLOR_138 {168, 231,  46, 255}
#define COLOR_139 {  0, 181,  67, 255}
#define COLOR_140 {  6,  90, 181, 255}
#define COLOR_141 {117,  70, 101, 255}
#define COLOR_142 {255, 110,  89, 255}
#define COLOR_143 {255, 157, 129, 255}


class Graphics {
	uint8_t fontSpriteData[128 * 64];

	Color _paletteColors[144];

	PicoRam* _memory;

	void copySpriteToScreen(
		uint8_t spritebuffer[],
		int scr_x,
		int scr_y,
		int spr_x,
		int spr_y,
		int spr_w,
		int spr_h,
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

	void swap(int *x, int *y);
	void applyCameraToPoint(int *x, int *y);

	void sortPointsLtoR(int *x1, int *y1, int *x2, int *y2);

	void sortCoordsForRect(int *x1, int *y1, int *x2, int *y2);

	bool isOnScreen(int x, int y);
	bool isWithinClip(int x, int y);
	bool isXWithinClip(int x);
	bool isYWithinClip(int y);
	int clampXCoordToClip(int x);
	int clampYCoordToClip(int y);

	void _setPixelFromSprite(int x, int y, uint8_t col);
	void _setPixelFromPen(int x, int y);
	void _safeSetPixelFromPen(int x, int y);
	void _private_h_line (int x1, int x2, int y);
	void _private_v_line (int y1, int y2, int x);

	public:
	Graphics(std::string fontdata, PicoRam* memory);

	uint8_t* GetP8FrameBuffer();
	uint8_t* GetScreenPaletteMap();
	Color* GetPaletteColors();

	bool isColorTransparent(uint8_t color);
	uint8_t getDrawPalMappedColor(uint8_t color);
	uint8_t getScreenPalMappedColor(uint8_t color);

	void cls();
	void cls(uint8_t color);

	void pset(int x, int y);
	void pset(int x, int y, uint8_t col);
	uint8_t pget(int x, int y);

	uint8_t color();
	uint8_t color(uint8_t c);

	void line ();
	void line (uint8_t col);
	void line (int x1, int y1);
	void line (int x1, int y1, uint8_t col);
	void line (int x1, int y1, int x2, int y2);
	void line (int x1, int y1, int x2, int y2, uint8_t col);

	void tline(int x0, int y0, int x1, int y1, fix32 mx, fix32 my);
	void tline(int x0, int y0, int x1, int y1, fix32 mx, fix32 my, fix32 mdx, fix32 mdy);

	void circ(int ox, int oy);
	void circ(int ox, int oy, int r);
	void circ(int ox, int oy, int r, uint8_t col);
	void circfill(int ox, int oy);
	void circfill(int ox, int oy, int r);
	void circfill(int ox, int oy, int r, uint8_t col);

	void oval(int x0, int y0, int x1, int y1);
	void oval(int x0, int y0, int x1, int y1, uint8_t col);

	void ovalfill(int x0, int y0, int x1, int y1);
	void ovalfill(int x0, int y0, int x1, int y1, uint8_t col);

	void rect(int x1, int y1, int x2, int y2);
	void rect(int x1, int y1, int x2, int y2, uint8_t col);
	void rectfill(int x1, int y1, int x2, int y2);
	void rectfill(int x1, int y1, int x2, int y2, uint8_t col);

	fix32 fillp(fix32 pat);

	int print(std::string str);
	int print(std::string str, int x, int y);
	int print(std::string str, int x, int y, uint8_t c);

	void spr(
		int n,
		int x,
		int y,
		fix32 w,
		fix32 h,
		bool flip_x,
		bool flip_y);

	void sspr(
        int sx,
        int sy,
        int sw,
        int sh,
        int dx,
        int dy,
        int dw,
        int dh,
        bool flip_x,
        bool flip_y);

	bool fget(uint8_t n, uint8_t f);
	uint8_t fget(uint8_t n);
	void fset(uint8_t n, uint8_t f, bool v);
	void fset(uint8_t n, uint8_t v);

	uint8_t sget(uint8_t x, uint8_t y);
	void sset(uint8_t x, uint8_t y, uint8_t c);

	std::tuple<int16_t, int16_t> camera();
	std::tuple<int16_t, int16_t> camera(int16_t x, int16_t y);

	std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> clip();
	std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> clip(int x, int y, int w, int h);

	uint8_t mget(int celx, int cely);
	void mset(int celx, int cely, uint8_t snum);

	void map(int celx, int cely, int sx, int sy, int celw, int celh);
	void map(int celx, int cely, int sx, int sy, int celw, int celh, uint8_t layer);

	void pal();
	uint8_t pal(uint8_t c0, uint8_t c1, uint8_t p);
	void palt();
	bool palt(uint8_t c0, bool t);

	std::tuple<uint8_t, uint8_t> cursor();
	std::tuple<uint8_t, uint8_t> cursor(int x, int y);
	std::tuple<uint8_t, uint8_t> cursor(int x, int y, uint8_t col);

};


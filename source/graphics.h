#pragma once

#include <string>
#include <tuple>
#include "hostVmShared.h"
#include "PicoRam.h"
#include <fix32.h>
using namespace z8;


class Graphics {
	//deprecated
	uint8_t fontSpriteData[128 * 64];

	PicoRam* _memory;

	void copySpriteToScreen(
		uint8_t* spritebuffer,
		int scr_x,
		int scr_y,
		int spr_x,
		int spr_y,
		int spr_w,
		int spr_h,
		bool flip_x,
		bool flip_y);

	void copyStretchSpriteToScreen(
		uint8_t* spritebuffer,
		int spr_x,
		int spr_y,
		int spr_w,
		int spr_h,
		int scr_x,
		int scr_y,
		int scr_w,
		int scr_h,
		bool flip_x,
		bool flip_y,
		bool skipStretchPx);

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
	uint8_t* GetP8SpriteSheetBuffer();
	uint8_t* GetScreenPaletteMap();

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

	int drawCharacter(
		uint8_t ch,
		int x,
		int y,
		uint8_t fgColor,
		uint8_t bgColor,
		uint8_t printMode = 0,
		int forceCharWidth = -1,
		int forceCharHeight = -1);
		
	std::tuple<int, int> drawCharacterFromBytes(
		uint8_t chBytes[],
		int x,
		int y,
		uint8_t fgColor,
		uint8_t bgColor,
		uint8_t printMode,
		uint8_t charWidth,
		uint8_t charHeight);

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
	void pal(uint8_t p);
	uint8_t pal(uint8_t c0, uint8_t c1, uint8_t p);
	void palt();
	bool palt(uint8_t c0, bool t);

	std::tuple<uint8_t, uint8_t> cursor();
	std::tuple<uint8_t, uint8_t> cursor(int x, int y);
	std::tuple<uint8_t, uint8_t> cursor(int x, int y, uint8_t col);

};


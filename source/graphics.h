#pragma once

#include <string>
#include "hostVmShared.h"

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


class Graphics {
	uint8_t _pico8_fb[128*128];
	uint8_t fontSpriteData[128 * 64];

	uint8_t spriteSheetData[128 * 64];
	uint8_t mapData[128 * 64];
	uint8_t spriteFlags[256];

	Color _paletteColors[16];

	uint8_t _gfxState_color;
    uint8_t _gfxState_bgColor;

    int _gfxState_text_x;
	int _gfxState_text_y;

	int _gfxState_camera_x;
	int _gfxState_camera_y;

	int _gfxState_clip_xb;
	int _gfxState_clip_yb;
	int _gfxState_clip_xe;
	int _gfxState_clip_ye;

	uint8_t _gfxState_drawPaletteMap[16];
	uint8_t _gfxState_screenPaletteMap[16];
	bool _gfxState_transparencyPalette[16];

	//not actually part of graphics state memory?
	int _gfxState_line_x;
	int _gfxState_line_y;
	bool _gfxState_line_valid;


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

	void _private_pset(int x, int y, uint8_t col);
	void _private_safe_pset(int x, int y, uint8_t col);
	void _private_h_line (int x1, int x2, int y, uint8_t col);
	void _private_v_line (int y1, int y2, int x, uint8_t col);

	public:
	Graphics(std::string fontdata);

	void setSpriteSheet(std::string spriteSheetString);
	void setSpriteFlags(std::string spriteFlagsString);
	void setMapData(std::string mapString);

	uint8_t* GetP8FrameBuffer();
	uint8_t* GetScreenPaletteMap();
	Color* GetPaletteColors();

	void cls();
	void cls(uint8_t color);

	void pset(int x, int y);
	void pset(int x, int y, uint8_t col);
	uint8_t pget(int x, int y);

	void color(uint8_t c);

	void line ();
	void line (uint8_t col);
	void line (int x1, int y1);
	void line (int x1, int y1, uint8_t col);
	void line (int x1, int y1, int x2, int y2);
	void line (int x1, int y1, int x2, int y2, uint8_t col);

	void circ(int ox, int oy);
	void circ(int ox, int oy, int r);
	void circ(int ox, int oy, int r, uint8_t col);
	void circfill(int ox, int oy);
	void circfill(int ox, int oy, int r);
	void circfill(int ox, int oy, int r, uint8_t col);

	void rect(int x1, int y1, int x2, int y2);
	void rect(int x1, int y1, int x2, int y2, uint8_t col);
	void rectfill(int x1, int y1, int x2, int y2);
	void rectfill(int x1, int y1, int x2, int y2, uint8_t col);

	int print(std::string str);
	int print(std::string str, int x, int y);
	int print(std::string str, int x, int y, uint16_t c);

	void spr(
		int n,
		int x,
		int y,
		double w,
		double h,
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

	void camera();
	void camera(int x, int y);

	void clip();
	void clip(int x, int y, int w, int h);

	uint8_t mget(int celx, int cely);
	void mset(int celx, int cely, uint8_t snum);

	void map(int celx, int cely, int sx, int sy, int celw, int celh);
	void map(int celx, int cely, int sx, int sy, int celw, int celh, uint8_t layer);

	void pal();
	void pal(uint8_t c0, uint8_t c1, uint8_t p);
	void palt();
	void palt(uint8_t c0, bool t);

	void cursor();
	void cursor(int x, int y);
	void cursor(int x, int y, uint8_t col);

};


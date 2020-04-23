#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "graphics.h"

#include "stringToDataHelpers.h"

#include "logger.h"

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


//call initialize to make sure defaults are correct
Graphics::Graphics(std::string fontdata) {
	this->_gfxState_color = 7;

	copy_data_to_sprites(fontSpriteData, fontdata);
}

void Graphics::setSpriteSheet(std::string spritesheetstring){
	Logger::Write("Copying data to spritesheet\n");
	copy_data_to_sprites(spriteSheetData, spritesheetstring);
}

void Graphics::setSpriteFlags(std::string spriteFlagsstring){
	Logger::Write("Copying data to sprite flags\n");
	copy_data_to_sprite_flags(spriteFlags, spriteFlagsstring);
}

//start helper methods
//based on tac08 implementation of blitter()
void Graphics::copySpriteToScreen(
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
		uint8_t* spr = spritebuffer + ((spr_y + y * dy) & 0x7f) * 64;

		for (short x = 0; x < scr_w; x++) {
			short combinedPixIdx = spr_x / 2 + x / 2;
			uint8_t bothPix = spr[combinedPixIdx];

			uint8_t c = x % 2 == 0 
				? bothPix & 0x0f //just first 4 bits
				: bothPix >> 4;  //just last 4 bits
				
			if (c != 0) { //if not transparent. Come back later to add palt() support by checking tranparency palette
				_private_pset(scr_x + x, scr_y + y, c); //set color on framebuffer. Come back later and add pal() by translating color
			}
		}
	}
}

//based on tac08 implementation of stretch_blitter()
//uses ints so we can shift bits and do integer division instead of floating point
void Graphics::copyStretchSpriteToScreen(
	uint8_t spritebuffer[],
	int spr_x,
	int spr_y,
	int spr_w,
	int spr_h,
	int scr_x,
	int scr_y,
	int scr_w,
	int scr_h,
	bool flip_x = false,
	bool flip_y = false) 
{
	if (false || (spr_h == scr_h && spr_w == scr_w)) {
		// use faster non stretch blitter if sprite is not stretched
		copySpriteToScreen(spritebuffer, scr_x, scr_y, spr_x, spr_y, scr_w, scr_h, flip_x, flip_y);
		return;
	}

	//shift bits to avoid floating point math
	spr_x = spr_x << 16;
	spr_y = spr_y << 16;
	spr_w = spr_w << 16;
	spr_h = spr_h << 16;

	int dx = spr_w / scr_w;
	int dy = spr_h / scr_h;

	if (flip_y) {
		spr_y += spr_h - 1 * dy;
		dy = -dy;
	}

	for (int y = 0; y < scr_h; y++) {
		uint8_t* spr = spritebuffer + (((spr_y + y * dy) >> 16) & 0x7f) * 64;

		if (!flip_x) {
			for (int x = 0; x < scr_w; x++) {
				int pixIndex = (spr_x + x * dx);
				int combinedPixIdx = ((pixIndex / 2) >> 16) & 0x7f;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = (pixIndex >> 16) % 2 == 0 
					? bothPix & 0x0f //just first 4 bits
					: bothPix >> 4;  //just last 4 bits
				if (c != 0) {
					_private_pset(scr_x + x, scr_y + y, c);
				}
			}
		} else {
			for (int x = 0; x < scr_w; x++) {
				int pixIndex = (spr_x + spr_w - (x + 1) * dx);
				int combinedPixIdx = ((pixIndex / 2) >> 16) & 0x7f;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = (pixIndex >> 16) % 2 == 0 
					? bothPix & 0x0f //just first 4 bits
					: bothPix >> 4;  //just last 4 bits
				if (c != 0) {
					_private_pset(scr_x + x, scr_y + y, c);
				}
			}
		}
	}
}

void Graphics::swap(short *x, short *y) {
   short temp;
   temp = *x;
   *x = *y;
   *y = temp;
  
   return;
}

void Graphics::sortPointsLtoR(short *x1, short *y1, short *x2, short *y2){
	if (*x1 > *x2) {
		swap(x1, x2);
		swap(y1, y2);
	}
}

void Graphics::sortCoordsForRect(short *x1, short *y1, short *x2, short *y2){
	if (*x1 > *x2) {
		swap(x1, x2);
	}

	if (*y1 > *y2) {
		swap(y1, y2);
	}
}

bool Graphics::isOnScreen(short x, short y) {
	return x >= 0 && x <= 127 && y >= 0 && y <= 127;
}
//end helper methods

void Graphics::cls() {
	this->cls(0);
}

void Graphics::cls(uint8_t color) {
	memset(_pico8_fb, color, sizeof(_pico8_fb));

	_gfxState_text_x = 0;
	_gfxState_text_y = 0;
}

void Graphics::pset(short x, short y){
	this->pset(x, y, _gfxState_color);
}

void Graphics::pset(short x, short y, uint8_t col){
	color(col);

	_private_pset(x, y, col);
}

void Graphics::_private_pset(short x, short y, uint8_t col) {
	if (isOnScreen(x, y)){
		_pico8_fb[(x * 128) + y] = col;
	}
}

uint8_t Graphics::pget(short x, short y){
	if (isOnScreen(x, y)){
		return _pico8_fb[(x * 128) + y];
	}

	return 0;
}

void Graphics::color(uint8_t col){
	this->_gfxState_color = col;
}

void Graphics::line () {
	//just invalidate line state
	this->_gfxState_line_x = 0;
	this->_gfxState_line_y = 0;
	this->_gfxState_line_valid = false;
}

void Graphics::line (uint8_t col){
	color(col);

	this->line();
}

void Graphics::line (short x1, short y1){
	if (this->_gfxState_line_valid){
		this->line(_gfxState_line_x, _gfxState_line_y, x1, y1, this->_gfxState_color);
	}
}

void Graphics::line (short x1, short y1, uint8_t col){
	if (this->_gfxState_line_valid){
		this->line(_gfxState_line_x, _gfxState_line_y, x1, y1, col);
	}
}

void Graphics::line (short x1, short y1, short x2, short y2){
	this->line(_gfxState_line_x, _gfxState_line_y, x1, y1, this->_gfxState_color);
}

void Graphics::line (short x1, short y1, short x2, short y2, uint8_t col) {
	this->_gfxState_line_x = x2;
	this->_gfxState_line_y = y2;
	this->_gfxState_line_valid = true;

	sortPointsLtoR(&x1, &y1, &x2, &y2);

	float run = x2 - x1;
	float rise = y2 - y1;

	//vertical line
	if (run == 0) {
		if (y1 > y2) {
			swap(&y1, &y2);
		}

		for (short y = y1; y <= y2; y++){
			_private_pset(x1, y, col);
		}
	}
	else {
		float slope = rise / run;

		for (short x = x1; x <= x2; x++){
			short y = y1 + (short)ceil((float)x * slope);
			_private_pset(x, y, col);
		}
	}
}

void Graphics::circ(short ox, short oy){
	this->circ(ox, oy, 4);
}

void Graphics::circ(short ox, short oy, short r){
	this->circ(ox, oy, r, this->_gfxState_color);
}

void Graphics::circ(short ox, short oy, short r, uint8_t col){
	color(col);
	short x = r;
	short y = 0;
	short decisionOver2 = 1-x;

	while (y <= x) {
		_private_pset(ox + x, oy + y, col);
		_private_pset(ox + y, oy + x, col);
		_private_pset(ox - x, oy + y, col);
		_private_pset(ox - y, oy + x, col);

		_private_pset(ox - x, oy - y, col);
		_private_pset(ox - y, oy - x, col);
		_private_pset(ox + x, oy - y, col);
		_private_pset(ox + y, oy - x, col);

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

void Graphics::circfill(short ox, short oy){
	this->circfill(ox, oy, 4);
}

void Graphics::circfill(short ox, short oy, short r){
	this->circfill(ox, oy, r, this->_gfxState_color);
}

void Graphics::circfill(short ox, short oy, short r, uint8_t col){
	color(col);
	if (r == 0) {
		_private_pset(ox, oy, col);
	}
	else if (r == 1) {
		_private_pset(ox, oy - 1, col);
		line(ox-1, oy, ox+1, oy, col);
		_private_pset(ox, oy + 1, col);
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

void Graphics::rect(short x1, short y1, short x2, short y2, uint8_t col) {
	sortCoordsForRect(&x1, &y1, &x2, &y2);

	for (short i = x1; i <= x2; i++) {
		for (short j = y1; j <= y2; j++) {
			if ((i == x1 || i == x2 || j == y1 || j == y2) ) {
				_private_pset(i, j, col);
			}
		}
	}

}

void Graphics::rectfill(short x1, short y1, short x2, short y2, uint8_t col) {
	sortCoordsForRect(&x1, &y1, &x2, &y2);

	for (short i = x1; i <= x2; i++) {
		for (short j = y1; j <= y2; j++) {
			_private_pset(i, j, col);
		}
	}
}

//based on tac08 impl
short Graphics::print(std::string str, short x, short y, uint16_t c) {
	_gfxState_text_x = x;

	for (size_t n = 0; n < str.length(); n++) {
		uint8_t ch = str[n];
		if (ch >= 0x10 && ch < 0x80) {
			short index = ch - 0x10;
			copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8, 4, 5);
			x += 4;
		} else if (ch >= 0x80) {
			short index = ch - 0x80;
			copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8 + 56, 8, 5);
			x += 8;
		} else if (ch == '\n') {
			x = _gfxState_text_x;
			y += 6;
		}
	}

	_gfxState_text_x = 0;
	_gfxState_text_y = y + 6;

	return x;
}

void Graphics::spr(short n, short x, short y, short w, short h, bool flip_x, bool flip_y) {
	short spr_x = (n % 16) * 8;
	short spr_y = (n / 16) * 8;
	copySpriteToScreen(spriteSheetData, x, y, spr_x, spr_y, w * 8, h * 8, flip_x, flip_y);
}

void Graphics::sspr(
        short sx,
        short sy,
        short sw,
        short sh,
        short dx,
        short dy,
        short dw,
        short dh,
        bool flip_x,
        bool flip_y)
{
	copyStretchSpriteToScreen(spriteSheetData, sx, sy, sw, sh, dx, dy, dw, dh, flip_x, flip_y);
}

bool Graphics::fget(uint8_t n, uint8_t f){
	return (this->fget(n) >> f) & 1;
}

uint8_t Graphics::fget(uint8_t n){
	return spriteFlags[n];
}

void Graphics::fset(uint8_t n, uint8_t f, bool v){
	if (v) {
		fset(n, fget(n) | (1 << f));
	}
	else {
		fset(n, fget(n) & ~(1 << f));
	}
}

void Graphics::fset(uint8_t n, uint8_t v){
	spriteFlags[n] = v;
}

uint8_t Graphics::sget(uint8_t x, uint8_t y){
	short combinedIdx = y * 64 + (x / 2);

	uint8_t combinedPix = this->spriteSheetData[combinedIdx];

	uint8_t c = x % 2 == 0 
		? combinedPix & 0x0f //just first 4 bits
		: combinedPix >> 4;  //just last 4 bits
	
	return c;
}

void Graphics::sset(uint8_t x, uint8_t y, uint8_t c){
	short combinedIdx = y * 64 + (x / 2);

	uint8_t currentByte = this->spriteSheetData[combinedIdx];
	uint8_t mask;
	// set just 4 bits: https://stackoverflow.com/a/4439221
	if (x % 2 == 0) {
		mask = 0x0f;
	}
	else {
		c = c << 4;
		mask = 0xf0;
	}

	this->spriteSheetData[combinedIdx] = (currentByte & ~mask) | (c & mask);
}

void Graphics::flipBuffer(uint8_t* fb) {
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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

#include "graphics.h"

#include "stringToDataHelpers.h"

#include "logger.h"

const uint8_t PicoScreenWidth = 128;
const uint8_t PicoScreenHeight = 128;

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

	copy_string_to_sprite_memory(fontSpriteData, fontdata);

	//make all colors default
	this->clip();
	this->pal();
}

void Graphics::setSpriteSheet(std::string spritesheetstring){
	Logger::Write("Copying data to spritesheet\n");
	copy_string_to_sprite_memory(spriteSheetData, spritesheetstring);
}

void Graphics::setSpriteFlags(std::string spriteFlagsstring){
	Logger::Write("Copying data to sprite flags\n");
	copy_string_to_memory(spriteFlags, spriteFlagsstring);
}

void Graphics::setMapData(std::string mapDataString){
	Logger::Write("Copying data to sprite flags\n");
	copy_string_to_memory(mapData, mapDataString);
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
	bool flip_x,
	bool flip_y) 
{

	//note: no clipping yet
	short scr_w = spr_w;
	short scr_h = spr_h;

	applyCameraToPoint(&scr_x, &scr_y);

	// left clip
	if (scr_x < _gfxState_clip_xb) {
		int nclip = _gfxState_clip_xb - scr_x;
		scr_x = _gfxState_clip_xb;
		scr_w -= nclip;
		if (!flip_x) {
			spr_x += nclip;
		} else {
			spr_w -= nclip;
		}
	}

	// right clip
	if (scr_x + scr_w > _gfxState_clip_xe) {
		int nclip = (scr_x + scr_w) - _gfxState_clip_xe;
		scr_w -= nclip;
	}

	// top clip
	if (scr_y < _gfxState_clip_yb) {
		int nclip = _gfxState_clip_yb - scr_y;
		scr_y = _gfxState_clip_yb;
		scr_h -= nclip;
		if (!flip_y) {
			spr_y += nclip;
		} else {
			spr_h -= nclip;
		}
	}

	// bottom clip
	if (scr_y + scr_h > _gfxState_clip_ye) {
		int nclip = (scr_y + scr_h) - _gfxState_clip_ye;
		scr_h -= nclip;
	}
	
	short dy = 1;
	if (flip_y) {
		spr_y += spr_h - 1;
		dy = -dy;
	}

	//todo: honor x and y flipping

	for (short y = 0; y < scr_h; y++) {
		uint8_t* spr = spritebuffer + ((spr_y + y * dy) & 0x7f) * 64;

		if (!flip_x) {
			for (short x = 0; x < scr_w; x++) {
				short combinedPixIdx = spr_x / 2 + x / 2;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = x % 2 == 0 
					? bothPix & 0x0f //just first 4 bits
					: bothPix >> 4;  //just last 4 bits
					
				if (_gfxState_transparencyPalette[c] == false) { //if not transparent. Come back later to add palt() support by checking tranparency palette
					_private_pset(scr_x + x, scr_y + y, c); //set color on framebuffer. Come back later and add pal() by translating color
				}
			}
		} else {
			for (short x = 0; x < scr_w; x++) {
				int pixIndex = spr_x + spr_w - (x + 1);
				short combinedPixIdx = pixIndex / 2;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = x % 2 == 0 
					? bothPix & 0x0f //just first 4 bits
					: bothPix >> 4;  //just last 4 bits
					
				if (_gfxState_transparencyPalette[c] == false) { //if not transparent. Come back later to add palt() support by checking tranparency palette
					_private_pset(scr_x + x, scr_y + y, c); //set color on framebuffer. Come back later and add pal() by translating color
				}
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
	bool flip_x,
	bool flip_y) 
{
	if (false || (spr_h == scr_h && spr_w == scr_w)) {
		// use faster non stretch blitter if sprite is not stretched
		copySpriteToScreen(spritebuffer, scr_x, scr_y, spr_x, spr_y, scr_w, scr_h, flip_x, flip_y);
		return;
	}

	applyCameraToPoint(&scr_x, &scr_y);

	//shift bits to avoid floating point math
	spr_x = spr_x << 16;
	spr_y = spr_y << 16;
	spr_w = spr_w << 16;
	spr_h = spr_h << 16;

	int dx = spr_w / scr_w;
	int dy = spr_h / scr_h;

	// left clip
	if (scr_x < _gfxState_clip_xb) {
		int nclip = _gfxState_clip_xb - scr_x;
		scr_x = _gfxState_clip_xb;
		scr_w -= nclip;
		if (!flip_x) {
			spr_x += nclip * dx;
		} else {
			spr_w -= nclip * dx;
		}
	}

	// right clip
	if (scr_x + scr_w > _gfxState_clip_xe) {
		int nclip = (scr_x + scr_w) - _gfxState_clip_xe;
		scr_w -= nclip;
	}

	// top clip
	if (scr_y < _gfxState_clip_yb) {
		int nclip = _gfxState_clip_yb - scr_y;
		scr_y = _gfxState_clip_yb;
		scr_h -= nclip;
		if (!flip_y) {
			spr_y += nclip * dy;
		} else {
			spr_h -= nclip * dy;
		}
	}

	// bottom clip
	if (scr_y + scr_h > _gfxState_clip_ye) {
		int nclip = (scr_y + scr_h) - _gfxState_clip_ye;
		scr_h -= nclip;
	}

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
				if (_gfxState_transparencyPalette[c] == false) {
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
				if (_gfxState_transparencyPalette[c] == false) {
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
}

void Graphics::applyCameraToPoint(short *x, short *y) {
	*x -= _gfxState_camera_x;
	*y -= _gfxState_camera_y;
}

void Graphics::applyCameraToPoint(int *x, int *y) {
	*x -= _gfxState_camera_x;
	*y -= _gfxState_camera_y;
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
	return 
		x >= 0 && 
		x <= 127 && 
		y >= 0 && 
		y <= 127;
}

bool Graphics::isWithinClip(short x, short y) {
	return 
		x >= _gfxState_clip_xb && 
		x <= _gfxState_clip_xe && 
		y >= _gfxState_clip_yb && 
		y <= _gfxState_clip_ye;
}

bool Graphics::isXWithinClip(short x) {
	return 
		x >= _gfxState_clip_xb && 
		x <= _gfxState_clip_xe;
}

bool Graphics::isYWithinClip(short y) {
	return 
		y >= _gfxState_clip_yb && 
		y <= _gfxState_clip_ye;
}


short clampCoordToScreenDims(short val) {
	return std::clamp(val, (short)0, (short)127);
}


void Graphics::_private_safe_pset(short x, short y, uint8_t col) {
	if (isWithinClip(x, y)){
		_pico8_fb[(x * 128) + y] = _gfxState_drawPaletteMap[col];
	}
}

void Graphics::_private_pset(short x, short y, uint8_t col) {
	x = x & 127;
	y = y & 127;

	_pico8_fb[(x * 128) + y] = _gfxState_drawPaletteMap[col];
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

	applyCameraToPoint(&x, &y);

	if (isWithinClip(x, y)){
		_private_pset(x, y, col);
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

void Graphics::_private_h_line (short x1, short x2, short y, uint8_t col){
	if (!isYWithinClip(y)){
		return;
	}

	short maxx = clampCoordToScreenDims(std::max(x1, x2));
	short minx = clampCoordToScreenDims(std::min(x1, x2));

	
	//possible todo: check if memset is any better here? this seems to be wrong
	//uint8_t* fb_line = _pico8_fb + y * PicoScreenWidth;
	//memset(fb_line + minx, col, maxx - minx);
	for (short x = minx; x <= maxx; x++){
		_private_pset(x, y, col);
	}
}

void Graphics::_private_v_line (short y1, short y2, short x, uint8_t col){
	//save draw calls if its out
	if (!isXWithinClip(x)){
		return;
	}

	short maxy = clampCoordToScreenDims(std::max(y1, y2));
	short miny = clampCoordToScreenDims(std::min(y1, y2));

	for (short y = miny; y <= maxy; y++){
		_private_pset(x, y, col);
	}
}

void Graphics::line(short x0, short y0, short x1, short y1, uint8_t col) {
	this->_gfxState_line_x = x1;
	this->_gfxState_line_y = y1;
	this->_gfxState_line_valid = true;

	applyCameraToPoint(&x0, &y0);
	applyCameraToPoint(&x1, &y1);

	color(col);

	//vertical line
	if (x0 == x1) {
		_private_v_line(y0, y1, x0, col);
	} 
	else if (y0 == y1) {
		_private_h_line(x0, x1, y0, col);
	}
	else {
		//tac08 line impl for diagonals (this should work for horizontal and vertical as well,
		//but it has more branching)
		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy, e2; /* error value e_xy */

		for (;;) { /* loop */
			_private_safe_pset(x0, y0, col);
			if (x0 == x1 && y0 == y1)
				break;
			e2 = 2 * err;
			if (e2 >= dy) {
				err += dy;
				x0 += sx;
			} /* e_xy+e_x > 0 */
			if (e2 <= dx) {
				err += dx;
				y0 += sy;
			} /* e_xy+e_y < 0 */
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

	applyCameraToPoint(&ox, &oy);

	short x = r;
	short y = 0;
	short decisionOver2 = 1-x;

	while (y <= x) {
		_private_safe_pset(ox + x, oy + y, col);
		_private_safe_pset(ox + y, oy + x, col);
		_private_safe_pset(ox - x, oy + y, col);
		_private_safe_pset(ox - y, oy + x, col);

		_private_safe_pset(ox - x, oy - y, col);
		_private_safe_pset(ox - y, oy - x, col);
		_private_safe_pset(ox + x, oy - y, col);
		_private_safe_pset(ox + y, oy - x, col);

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

	applyCameraToPoint(&ox, &oy);

	if (r == 0) {
		_private_safe_pset(ox, oy, col);
	}
	else if (r == 1) {
		_private_safe_pset(ox, oy - 1, col);
		_private_h_line(ox-1, ox+1, oy, col);
		_private_safe_pset(ox, oy + 1, col);
	}
	else if (r > 0) {
		short x = -r, y = 0, err = 2 - 2 * r;
		do {
			_private_h_line(ox - x, ox + x, oy + y, col);
			_private_h_line(ox - x, ox + x, oy - y, col);
			r = err;
			if (r > x)
				err += ++x * 2 + 1;
			if (r <= y)
				err += ++y * 2 + 1;
		} while (x < 0);
	}
	
}

void Graphics::rect(short x1, short y1, short x2, short y2) {
	this->rect(x1, y1, x2, y2, this->_gfxState_color);
}

void Graphics::rect(short x1, short y1, short x2, short y2, uint8_t col) {
	color(col);

	applyCameraToPoint(&x1, &y1);
	applyCameraToPoint(&x2, &y2);

	sortCoordsForRect(&x1, &y1, &x2, &y2);

	_private_h_line(x1, x2, y1, col);
	_private_h_line(x1, x2, y2, col);

	_private_v_line(y1, y2, x1, col);
	_private_v_line(y1, y2, x2, col);
}

void Graphics::rectfill(short x1, short y1, short x2, short y2) {
	this->rectfill(x1, y1, x2, y2, this->_gfxState_color);
}

void Graphics::rectfill(short x1, short y1, short x2, short y2, uint8_t col) {
	color(col);

	sortCoordsForRect(&x1, &y1, &x2, &y2);

	for (short y = y1; y <= y2; y++) {
		_private_h_line(x1, x2, y, col);
	}
}

short Graphics::print(std::string str) {
	short result = this->print(str, _gfxState_text_x, _gfxState_text_y);

	_gfxState_text_y += 6;

	return result;
}

short Graphics::print(std::string str, short x, short y) {
	return this->print(str, x, y, _gfxState_color);
}

//based on tac08 impl
short Graphics::print(std::string str, short x, short y, uint16_t c) {
	color(c);

	_gfxState_text_x = x;
	_gfxState_text_y = y;

	//font sprite sheet has text as color 7, with 0 as transparent. We need to override
	//these values and restore them after
	uint8_t prevCol7Map = _gfxState_drawPaletteMap[7];
	bool prevCol0Transp = _gfxState_transparencyPalette[0];

	_gfxState_drawPaletteMap[7] = c;
	_gfxState_transparencyPalette[0] = true;


	for (size_t n = 0; n < str.length(); n++) {
		uint8_t ch = str[n];
		if (ch >= 0x10 && ch < 0x80) {
			short index = ch - 0x10;
			copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8, 4, 5, false, false);
			x += 4;
		} else if (ch >= 0x80) {
			short index = ch - 0x80;
			copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8 + 56, 8, 5, false, false);
			x += 8;
		} else if (ch == '\n') {
			x = _gfxState_text_x;
			y += 6;
		}
	}

	_gfxState_drawPaletteMap[7] = prevCol7Map;
	_gfxState_transparencyPalette[0] = prevCol0Transp;

	//todo: auto scrolling

	return x;
}

void Graphics::spr(
	short n,
	short x,
	short y,
	double w = 1.0,
	double h = 1.0,
	bool flip_x = false,
	bool flip_y = false) 
{
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
        bool flip_x = false,
        bool flip_y = false)
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

void Graphics::camera() {
	this->camera(0, 0);
}

void Graphics::camera(short x, short y) {
	_gfxState_camera_x = x;
	_gfxState_camera_y = y;
}

void Graphics::clip() {
	this->clip(0, 0, 127, 127);
}

void Graphics::clip(short x, short y, short w, short h) {
	short xe = x + w;
	short ye = y + h;
	_gfxState_clip_xb = clampCoordToScreenDims(x);
	_gfxState_clip_yb = clampCoordToScreenDims(y);
	_gfxState_clip_xe = clampCoordToScreenDims(xe);
	_gfxState_clip_ye = clampCoordToScreenDims(ye);
}


//map methods heavily based on tac08 implementation
uint8_t Graphics::mget(short celx, short cely){
	if (cely < 32) {
		return this->mapData[cely * 128 + celx];
	}
	else if (cely < 64){
		return this->spriteSheetData[cely* 128 + celx];
	}

	return 0;
}

void Graphics::mset(short celx, short cely, uint8_t snum){
	if (cely < 32) {
		this->mapData[cely * 128 + celx] = snum;
	}
	else if (cely < 64){
		this->spriteSheetData[cely* 128 + celx] = snum;
	}
}

void Graphics::map(int celx, int cely, int sx, int sy, int celw, int celh) {
	map(celx, cely, sx, sy, celw, celh, 0);
}

void Graphics::map(int celx, int cely, int sx, int sy, int celw, int celh, uint8_t layer) {
	for (int y = 0; y < celh; y++) {
		for (int x = 0; x < celw; x++) {
			uint8_t cell = mget(celx + x, cely + y);
			if (cell && ((layer == 0) || (fget(cell) & layer))) {
				spr(cell, sx + x * 8, sy + y * 8);
			}
		}
	}
}

void Graphics::pal() {
	for (uint8_t c = 0; c < 16; c++) {
		_gfxState_drawPaletteMap[c] = c;
		_gfxState_screenPaletteMap[c] = c;
	}

	this->palt();
}

void Graphics::pal(uint8_t c0, uint8_t c1, uint8_t p){
	if (c0 < 16 && c1 < 16) {
		if (p == 0) {
			_gfxState_drawPaletteMap[c0] = c1;
		} else if (p == 1) {
			_gfxState_screenPaletteMap[c0] = c1;
		}
	}
}

void Graphics::palt() {
	for (uint8_t c = 0; c < 16; c++) {
		_gfxState_transparencyPalette[c] = c == 0 ? true : false;
	}
}

void Graphics::palt(uint8_t c, bool t){
	if (c < 16) {
		_gfxState_transparencyPalette[c] = t;
	}
}

void Graphics::flipBuffer(uint8_t* fb, int width, int height) {
	short x, y;

	short xOffset = width / 2 - PicoScreenWidth / 2;
	short yOffset = height / 2 - PicoScreenHeight / 2;
	//todo: test if it is faster to convert colors to uint24_ts and write one instead of 3 (assuming these are )
    for(x = 0; x < 128; x++) {
    	for(y = 0; y < 128; y++) {
			uint8_t c = _pico8_fb[x*128 + y];
			Color col = PaletteColors[_gfxState_screenPaletteMap[c]];

			int pixIdx = (((x + xOffset)*height)+ ((height - 1) - (y + yOffset)))*3;

			fb[pixIdx + 0] = col.Blue;
			fb[pixIdx + 1] = col.Green;
			fb[pixIdx + 2] = col.Red;
    	}
    }
}

void Graphics::cursor() {
	this->cursor(0, 0);
}

void Graphics::cursor(short x, short y) {
	_gfxState_text_x = x;
	_gfxState_text_y = y;
}

void Graphics::cursor(short x, short y, uint8_t col) {
	color(col);

	this->cursor(x, y);
}


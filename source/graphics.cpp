#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

#include "graphics.h"
#include "hostVmShared.h"
#include "nibblehelpers.h"

#include "stringToDataHelpers.h"

#include "logger.h"

#include <fix32.h>
using namespace z8;

const uint8_t PicoScreenWidth = 128;
const uint8_t PicoScreenHeight = 128;

const Color BgGray = BG_GRAY_COLOR;


//call initialize to make sure defaults are correct
Graphics::Graphics(std::string fontdata, PicoRam* memory) {
	_memory = memory;
	
	copy_string_to_sprite_memory(fontSpriteData, fontdata);

	_paletteColors[0] = COLOR_00;
	_paletteColors[1] = COLOR_01;
	_paletteColors[2] = COLOR_02;
	_paletteColors[3] = COLOR_03;
	_paletteColors[4] = COLOR_04;
	_paletteColors[5] = COLOR_05;
	_paletteColors[6] = COLOR_06;
	_paletteColors[7] = COLOR_07;
	_paletteColors[8] = COLOR_08;
	_paletteColors[9] = COLOR_09;
	_paletteColors[10] = COLOR_10;
	_paletteColors[11] = COLOR_11;
	_paletteColors[12] = COLOR_12;
	_paletteColors[13] = COLOR_13;
	_paletteColors[14] = COLOR_14;
	_paletteColors[15] = COLOR_15;

	for (int i = 16; i < 128; i++) {
		_paletteColors[i] = {0, 0, 0, 0};
	}

	_paletteColors[128] = COLOR_128;
	_paletteColors[129] = COLOR_129;
	_paletteColors[130] = COLOR_130;
	_paletteColors[131] = COLOR_131;
	_paletteColors[132] = COLOR_132;
	_paletteColors[133] = COLOR_133;
	_paletteColors[134] = COLOR_134;
	_paletteColors[135] = COLOR_135;
	_paletteColors[136] = COLOR_136;
	_paletteColors[137] = COLOR_137;
	_paletteColors[138] = COLOR_138;
	_paletteColors[139] = COLOR_139;
	_paletteColors[140] = COLOR_140;
	_paletteColors[141] = COLOR_141;
	_paletteColors[142] = COLOR_142;
	_paletteColors[143] = COLOR_143;

	//set default clip
	clip();
	pal();
	color();
}


uint8_t* Graphics::GetP8FrameBuffer(){
	//TODO: replace with ram's screen buffer
	return _memory->screenBuffer;
}

uint8_t* Graphics::GetScreenPaletteMap(){
	return _memory->drawState.screenPaletteMap;
}

Color* Graphics::GetPaletteColors(){
	return this->_paletteColors;
}

//start helper methods
//based on tac08 implementation of blitter()
void Graphics::copySpriteToScreen(
	uint8_t spritebuffer[],
	int scr_x,
	int scr_y,
	int spr_x,
	int spr_y,
	int spr_w,
	int spr_h,
	bool flip_x,
	bool flip_y) 
{

	//note: no clipping yet
	int scr_w = spr_w;
	int scr_h = spr_h;

	applyCameraToPoint(&scr_x, &scr_y);

	// left clip
	if (scr_x < _memory->drawState.clip_xb) {
		int nclip = _memory->drawState.clip_xb - scr_x;
		scr_x = _memory->drawState.clip_xb;
		scr_w -= nclip;
		if (!flip_x) {
			spr_x += nclip;
		} else {
			spr_w -= nclip;
		}
	}

	// right clip
	if (scr_x + scr_w > _memory->drawState.clip_xe) {
		int nclip = (scr_x + scr_w) - _memory->drawState.clip_xe;
		scr_w -= nclip;
	}

	// top clip
	if (scr_y < _memory->drawState.clip_yb) {
		int nclip = _memory->drawState.clip_yb - scr_y;
		scr_y = _memory->drawState.clip_yb;
		scr_h -= nclip;
		if (!flip_y) {
			spr_y += nclip;
		} else {
			spr_h -= nclip;
		}
	}

	// bottom clip
	if (scr_y + scr_h > _memory->drawState.clip_ye) {
		int nclip = (scr_y + scr_h) - _memory->drawState.clip_ye;
		scr_h -= nclip;
	}
	
	int dy = 1;
	if (flip_y) {
		spr_y += spr_h - 1;
		dy = -dy;
	}

	for (int y = 0; y < scr_h; y++) {
		uint8_t* spr = spritebuffer + ((spr_y + y * dy) & 0x7f) * 64;

		if (!flip_x) {
			for (int x = 0; x < scr_w; x++) {
				int abs_spr_x = spr_x + x;
				int combinedPixIdx = abs_spr_x / 2;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = abs_spr_x % 2 == 0 
					? bothPix & 0x0f //just first 4 bits
					: bothPix >> 4;  //just last 4 bits
					
				if (isColorTransparent(c) == false) { //if not transparent. Come back later to add palt() support by checking tranparency palette
					_private_pset(scr_x + x, scr_y + y, c); //set color on framebuffer. Come back later and add pal() by translating color
				}
			}
		} else {
			for (int x = 0; x < scr_w; x++) {
				int pixIndex = spr_x + spr_w - (x + 1);
				int combinedPixIdx = pixIndex / 2;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = x % 2 == 0 
					? bothPix >> 4 //just first 4 bits
					: bothPix & 0x0f;  //just last 4 bits
					
				if (isColorTransparent(c) == false) { //if not transparent. Come back later to add palt() support by checking tranparency palette
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
	if (spr_h == scr_h && spr_w == scr_w) {
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
	if (scr_x < _memory->drawState.clip_xb) {
		int nclip = _memory->drawState.clip_xb - scr_x;
		scr_x = _memory->drawState.clip_xb;
		scr_w -= nclip;
		if (!flip_x) {
			spr_x += nclip * dx;
		} else {
			spr_w -= nclip * dx;
		}
	}

	// right clip
	if (scr_x + scr_w > _memory->drawState.clip_xe) {
		int nclip = (scr_x + scr_w) - _memory->drawState.clip_xe;
		scr_w -= nclip;
	}

	// top clip
	if (scr_y < _memory->drawState.clip_yb) {
		int nclip = _memory->drawState.clip_yb - scr_y;
		scr_y = _memory->drawState.clip_yb;
		scr_h -= nclip;
		if (!flip_y) {
			spr_y += nclip * dy;
		} else {
			spr_h -= nclip * dy;
		}
	}

	// bottom clip
	if (scr_y + scr_h > _memory->drawState.clip_ye) {
		int nclip = (scr_y + scr_h) - _memory->drawState.clip_ye;
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
				if (isColorTransparent(c) == false) {
					_private_pset(scr_x + x, scr_y + y, c);
				}
			}
		} else {
			for (int x = 0; x < scr_w; x++) {
				int pixIndex = (spr_x + spr_w - (x + 1) * dx);
				int combinedPixIdx = ((pixIndex / 2) >> 16) & 0x7f;
				uint8_t bothPix = spr[combinedPixIdx];

				uint8_t c = (pixIndex >> 16) % 2 == 0 
					? bothPix >> 4 //just first 4 bits
					: bothPix & 0x0f;  //just last 4 bits
				if (isColorTransparent(c) == false) {
					_private_pset(scr_x + x, scr_y + y, c);
				}
			}
		}
	}
}

void Graphics::swap(int *x, int *y) {
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

void Graphics::applyCameraToPoint(int *x, int *y) {
	*x -= _memory->drawState.camera_x;
	*y -= _memory->drawState.camera_y;
}

void Graphics::sortPointsLtoR(int *x1, int *y1, int *x2, int *y2){
	if (*x1 > *x2) {
		swap(x1, x2);
		swap(y1, y2);
	}
}

void Graphics::sortCoordsForRect(int *x1, int *y1, int *x2, int *y2){
	if (*x1 > *x2) {
		swap(x1, x2);
	}

	if (*y1 > *y2) {
		swap(y1, y2);
	}
}

bool Graphics::isOnScreen(int x, int y) {
	return 
		x >= 0 && 
		x <= 127 && 
		y >= 0 && 
		y <= 127;
}

bool Graphics::isColorTransparent(uint8_t color) {
	color = color & 0x0f;
	return (_memory->drawState.drawPaletteMap[color] >> 4) & 1U; //4th bit is transparency
}

uint8_t Graphics::getDrawPalMappedColor(uint8_t color) {
	color = color & 0x0f;
	return _memory->drawState.drawPaletteMap[color] & 0x0f; //bits 0-3 are color
}

uint8_t Graphics::getScreenPalMappedColor(uint8_t color) {
	color = color & 0x0f;
	return _memory->drawState.screenPaletteMap[color] & 0x8f;
}

bool Graphics::isWithinClip(int x, int y) {
	return 
		x >= _memory->drawState.clip_xb && 
		x < _memory->drawState.clip_xe && 
		y >= _memory->drawState.clip_yb && 
		y < _memory->drawState.clip_ye;
}

bool Graphics::isXWithinClip(int x) {
	return 
		x >= _memory->drawState.clip_xb && 
		x < _memory->drawState.clip_xe;
}

bool Graphics::isYWithinClip(int y) {
	return 
		y >= _memory->drawState.clip_yb && 
		y < _memory->drawState.clip_ye;
}


int clampCoordToScreenDims(int val) {
	return std::clamp(val, 0, 128);
}

int Graphics::clampXCoordToClip(int x) {
	return std::clamp(
		x,
		(int)_memory->drawState.clip_xb,
		(int)_memory->drawState.clip_xe - 1);
}

int Graphics::clampYCoordToCLip(int y) {
	return std::clamp(
		y,
		(int)_memory->drawState.clip_yb,
		(int)_memory->drawState.clip_ye - 1);
}


void Graphics::_private_safe_pset(int x, int y, uint8_t col) {
	if (isWithinClip(x, y)){
		_private_pset(x, y, col);
	}
}

void Graphics::_private_pset(int x, int y, uint8_t col) {
	x = x & 127;
	y = y & 127;

	col = getDrawPalMappedColor(col);

	setPixelNibble(x, y, col, _memory->screenBuffer);
}
//end helper methods

void Graphics::cls() {
	this->cls(0);
}

void Graphics::cls(uint8_t color) {
	color = color & 15;
	uint8_t val = color << 4 | color;
	memset(_memory->screenBuffer, val, sizeof(_memory->screenBuffer));

	_memory->drawState.text_x = 0;
	_memory->drawState.text_y = 0;
}

void Graphics::pset(int x, int y){
	this->pset(x, y, _memory->drawState.color);
}

void Graphics::pset(int x, int y, uint8_t col){
	color(col);

	applyCameraToPoint(&x, &y);

	if (isWithinClip(x, y)){
		_private_pset(x, y, col);
	}
}

uint8_t Graphics::pget(int x, int y){
	applyCameraToPoint(&x, &y);

	if (isOnScreen(x, y)){
		return getPixelNibble(x, y, _memory->screenBuffer);
	}

	return 0;
}

void Graphics::color(){
	color(6);
}

void Graphics::color(uint8_t col){
	this->_memory->drawState.color = col;
}

void Graphics::line () {
	//just invalidate line state
	this->_memory->drawState.line_x = 0;
	this->_memory->drawState.line_y = 0;
	this->_memory->drawState.lineInvalid = 1;
}

void Graphics::line (uint8_t col){
	color(col);

	this->line();
}

void Graphics::line (int x1, int y1){
	if (this->_memory->drawState.lineInvalid == false){
		this->line(_memory->drawState.line_x, _memory->drawState.line_y, x1, y1, _memory->drawState.color);
	}
}

void Graphics::line (int x1, int y1, uint8_t col){
	if (_memory->drawState.lineInvalid == false){
		this->line(_memory->drawState.line_x, _memory->drawState.line_y, x1, y1, col);
	}
}

void Graphics::line (int x1, int y1, int x2, int y2){
	this->line(x1, y1, x2, y2, _memory->drawState.color);
}

void Graphics::_private_h_line (int x1, int x2, int y, uint8_t col){
	if (!isYWithinClip(y)){
		return;
	}

	int maxx = clampXCoordToClip(std::max(x1, x2));
	int minx = clampXCoordToClip(std::min(x1, x2));

	for (int x = minx; x <= maxx; x++){
		_private_pset(x, y, col);
	}
}

void Graphics::_private_v_line (int y1, int y2, int x, uint8_t col){
	//save draw calls if its out
	if (!isXWithinClip(x)){
		return;
	}

	int maxy = clampYCoordToCLip(std::max(y1, y2));
	int miny = clampYCoordToCLip(std::min(y1, y2));

	for (int y = miny; y <= maxy; y++){
		_private_pset(x, y, col);
	}
}

void Graphics::line(int x0, int y0, int x1, int y1, uint8_t col) {
	_memory->drawState.line_x = x1;
	_memory->drawState.line_y = y1;
	_memory->drawState.lineInvalid = 0;

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


void Graphics::tline(int x0, int y0, int x1, int y1, fix32 mx, fix32 my){
	tline(
		x0,
		y0,
		x1,
		y1,
		mx,
		my,
		fix32::frombits(0x2000), // 1/8
		fix32(0)
	);
}

//proted from zepto 8 impl
void Graphics::tline(int x0, int y0, int x1, int y1, fix32 mx, fix32 my, fix32 mdx, fix32 mdy){
	applyCameraToPoint(&x0, &y0);
	applyCameraToPoint(&x1, &y1);

	//determine whether x or y coordinates need to get incremented
	bool xDifGreater = std::abs(x1 - x0) >= std::abs(y1 - y0);
	int dx = xDifGreater ? x0 <= x1 ? 1 : -1 : 0;
    int dy = xDifGreater ? 0 : y0 <= y1 ? 1 : -1;

	bool vertical = x0 == x1;

	int x = clampCoordToScreenDims(x0);
	int xend = clampCoordToScreenDims(x1);
	int y = clampCoordToScreenDims(y0);
	int yend = clampCoordToScreenDims(y1);

	auto &ds = _memory->drawState;

	// Retrieve masks for wrap-around and subtract 0x0.0001
	fix32 xmask = fix32(ds.tlineMapWidth) - fix32::frombits(1);
    fix32 ymask = fix32(ds.tlineMapHeight) - fix32::frombits(1);

	// Advance texture coordinates; do it in steps to avoid overflows
    int delta = abs(xDifGreater ? x - x0 : y - y0);
    while (delta) {
        int step = std::min(8192, delta);
        mx = (mx & ~xmask) | ((mx + mdx * fix32(step)) & xmask);
        my = (my & ~ymask) | ((my + mdy * fix32(step)) & ymask);
        delta -= step;
    }

	for (;;) {
        // Find sprite in map memory
        int sx = (ds.tlineMapXOffset + int(mx)) & 0x7f;
        int sy = (ds.tlineMapYOffset + int(my)) & 0x3f;
		uint8_t sprite = mget(sx, sy);
        //uint8_t bits = fget(sprite);

		int spr_x = (sprite % 16) * 8;
		int spr_y = (sprite / 16) * 8;

        // If found, draw pixel //todo layer param
		//if (cell && ((layer == 0) || (fget(cell) & layer))) {
        if (sprite) {
            //uint8_t col = _memory->spriteSheetData.gfx.get(spr_x + (int(mx << 3) & 0x7),
            //                        spr_y + (int(my << 3) & 0x7));
			uint8_t col = getPixelNibble(
				spr_x + (int(mx << 3) & 0x7),
				spr_y + (int(my << 3) & 0x7),
				_memory->spriteSheetData);

            if (!isColorTransparent(col)) {
                _private_pset(x, y, getDrawPalMappedColor(col));
            }
        }

        // Advance source coordinates
        mx = (mx & ~xmask) | ((mx + mdx) & xmask);
        my = (my & ~ymask) | ((my + mdy) & ymask);

        // Advance destination coordinates
        if (xDifGreater) {
            if (x == xend)
                break;
            x += dx;
            y = y0 + ((float)(x - x0) / (x1 - x0)) * (y1 - y0);
        }
        else {
            if (y == yend)
                break;
            y += dy;
			if (! vertical) {
            	x = x0 + ((float)(y - y0) / (y1 - y0)) * (x1 - x0);
			}
		}
	}
}

void Graphics::circ(int ox, int oy){
	this->circ(ox, oy, 4);
}

void Graphics::circ(int ox, int oy, int r){
	this->circ(ox, oy, r, _memory->drawState.color);
}

void Graphics::circ(int ox, int oy, int r, uint8_t col){
	color(col);

	applyCameraToPoint(&ox, &oy);

	int x = r;
	int y = 0;
	int decisionOver2 = 1-x;

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

void Graphics::circfill(int ox, int oy){
	this->circfill(ox, oy, 4);
}

void Graphics::circfill(int ox, int oy, int r){
	this->circfill(ox, oy, r, _memory->drawState.color);
}

void Graphics::circfill(int ox, int oy, int r, uint8_t col){
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
		int x = -r, y = 0, err = 2 - 2 * r;
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

void Graphics::oval(int x0, int y0, int x1, int y1) {
	this->oval(x0, y0, x1, y1, _memory->drawState.color);
}

//https://stackoverflow.com/a/8448181
void Graphics::oval(int x0, int y0, int x1, int y1, uint8_t col) {
	color(col);

	applyCameraToPoint(&x0, &y0);
	applyCameraToPoint(&x1, &y1);

	sortCoordsForRect(&x0, &y0, &x1, &y1);

	//x radius and y radius
	int xr = (x1 - x0) / 2;
	int yr = (y1 - y0) / 2;

	//center location
	int xc = x0 + xr;
	int yc = y0 + yr;

	int wx, wy;
    int thresh;
    int asq = xr * xr;
    int bsq = yr * yr;
    int xa, ya;

	_private_safe_pset(xc, yc+yr, col);
    _private_safe_pset(xc, yc-yr, col);

    wx = 0;
    wy = yr;
    xa = 0;
    ya = asq * 2 * yr;
    thresh = asq / 4 - asq * yr;

    for (;;) {
        thresh += xa + bsq;

        if (thresh >= 0) {
            ya -= asq * 2;
            thresh -= ya;
            wy--;
        }

        xa += bsq * 2;
        wx++;

        if (xa >= ya)
          break;


        _private_safe_pset(xc+wx, yc-wy, col);
        _private_safe_pset(xc-wx, yc-wy, col);
        _private_safe_pset(xc+wx, yc+wy, col);
        _private_safe_pset(xc-wx, yc+wy, col);
    }

    _private_safe_pset(xc+xr, yc, col);
    _private_safe_pset(xc-xr, yc, col);

    wx = xr;
    wy = 0;
    xa = bsq * 2 * xr;

    ya = 0;
    thresh = bsq / 4 - bsq * xr;

    for (;;) {
        thresh += ya + asq;

        if (thresh >= 0) {
            xa -= bsq * 2;
            thresh = thresh - xa;
            wx--;
        }

        ya += asq * 2;
        wy++;

        if (ya > xa)
          break;

        _private_safe_pset(xc+wx, yc-wy, col);
        _private_safe_pset(xc-wx, yc-wy, col);
        _private_safe_pset(xc+wx, yc+wy, col);
        _private_safe_pset(xc-wx, yc+wy, col);
    }
}

void Graphics::ovalfill(int x0, int y0, int x1, int y1) {
	this->ovalfill(x0, y0, x1, y1, _memory->drawState.color);
}

//https://stackoverflow.com/a/8448181
void Graphics::ovalfill(int x0, int y0, int x1, int y1, uint8_t col){
	color(col);

	applyCameraToPoint(&x0, &y0);
	applyCameraToPoint(&x1, &y1);

	sortCoordsForRect(&x0, &y0, &x1, &y1);

	//x radius and y radius
	int xr = (x1 - x0) / 2;
	int yr = (y1 - y0) / 2;

	//center location
	int xc = x0 + xr;
	int yc = y0 + yr;

	int wx, wy;
    int thresh;
    int asq = xr * xr;
    int bsq = yr * yr;
    int xa, ya;

	_private_v_line(yc+yr, yc-yr, xc, col);

    wx = 0;
    wy = yr;
    xa = 0;
    ya = asq * 2 * yr;
    thresh = asq / 4 - asq * yr;

    for (;;) {
        thresh += xa + bsq;

        if (thresh >= 0) {
            ya -= asq * 2;
            thresh -= ya;
            wy--;
        }

        xa += bsq * 2;
        wx++;

        if (xa >= ya)
          break;

		_private_h_line(xc+wx, xc-wx, yc-wy, col);
		_private_h_line(xc+wx, xc-wx, yc+wy, col);
    }

	_private_h_line(xc+xr, xc-xr, yc, col);

    wx = xr;
    wy = 0;
    xa = bsq * 2 * xr;

    ya = 0;
    thresh = bsq / 4 - bsq * xr;

    for (;;) {
        thresh += ya + asq;

        if (thresh >= 0) {
            xa -= bsq * 2;
            thresh = thresh - xa;
            wx--;
        }

        ya += asq * 2;
        wy++;

        if (ya > xa)
          break;

		_private_h_line(xc+wx, xc-wx, yc-wy, col);
		_private_h_line(xc+wx, xc-wx, yc+wy, col);
    }
	/*
	//this algo doesn't like up correctly
	//center line
	_private_h_line(xc - xr, xc + xr, yc, col);

	for (int y = 1; y <= yr; y++) {
		int x = currWidth - (dx - 1);  // try slopes of dx - 1 or more
		for ( ; x > 0; x--) {
			if (x*x*hh + y*y*ww <= hhww) {
				break;
			}
		}
		dx = currWidth - x;  // current approximation of the slope
		currWidth = x;

		_private_h_line(xc - currWidth, xc + currWidth, yc + y, col);
		_private_h_line(xc - currWidth, xc + currWidth, yc - y, col);
	}
	*/
}

void Graphics::rect(int x1, int y1, int x2, int y2) {
	this->rect(x1, y1, x2, y2, _memory->drawState.color);
}

void Graphics::rect(int x1, int y1, int x2, int y2, uint8_t col) {
	color(col);

	applyCameraToPoint(&x1, &y1);
	applyCameraToPoint(&x2, &y2);

	sortCoordsForRect(&x1, &y1, &x2, &y2);

	_private_h_line(x1, x2, y1, col);
	_private_h_line(x1, x2, y2, col);

	_private_v_line(y1, y2, x1, col);
	_private_v_line(y1, y2, x2, col);
}

void Graphics::rectfill(int x1, int y1, int x2, int y2) {
	this->rectfill(x1, y1, x2, y2, _memory->drawState.color);
}

void Graphics::rectfill(int x1, int y1, int x2, int y2, uint8_t col) {
	color(col);

	applyCameraToPoint(&x1, &y1);
	applyCameraToPoint(&x2, &y2);

	sortCoordsForRect(&x1, &y1, &x2, &y2);

	for (int y = y1; y <= y2; y++) {
		_private_h_line(x1, x2, y, col);
	}
}

int Graphics::print(std::string str) {
	int result = this->print(str, _memory->drawState.text_x, _memory->drawState.text_y);

	_memory->drawState.text_y += 6;

	return result;
}

int Graphics::print(std::string str, int x, int y) {
	return this->print(str, x, y, _memory->drawState.color);
}

//based on tac08 impl
int Graphics::print(std::string str, int x, int y, uint16_t c) {
	color(c);

	_memory->drawState.text_x = x;
	_memory->drawState.text_y = y;

	//font sprite sheet has text as color 7, with 0 as transparent. We need to override
	//these values and restore them after
	uint8_t prevCol7Map = _memory->drawState.drawPaletteMap[7];
	bool prevCol0Transp = isColorTransparent(0);

	pal(7, c, 0);
	palt(7, false);
	palt(0, true);


	for (size_t n = 0; n < str.length(); n++) {
		uint8_t ch = str[n];
		if (ch >= 0x10 && ch < 0x80) {
			int index = ch - 0x10;
			copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8, 4, 5, false, false);
			x += 4;
		} else if (ch >= 0x80) {
			int index = ch - 0x80;
			copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8 + 56, 8, 5, false, false);
			x += 8;
		} else if (ch == '\n') {
			x = _memory->drawState.text_x;
			y += 6;
		}
	}

	_memory->drawState.drawPaletteMap[7] = prevCol7Map;
	palt(0, prevCol0Transp);

	//todo: auto scrolling

	return x;
}

void Graphics::spr(
	int n,
	int x,
	int y,
	fix32 w = 1.0,
	fix32 h = 1.0,
	bool flip_x = false,
	bool flip_y = false) 
{
	int spr_x = (n % 16) * 8;
	int spr_y = (n / 16) * 8;
	int16_t spr_w = (int16_t)(w * (fix32)8);
	int16_t spr_h = (int16_t)(h * (fix32)8);
	copySpriteToScreen(_memory->spriteSheetData, x, y, spr_x, spr_y, spr_w, spr_h, flip_x, flip_y);
}

void Graphics::sspr(
        int sx,
        int sy,
        int sw,
        int sh,
        int dx,
        int dy,
        int dw,
        int dh,
        bool flip_x = false,
        bool flip_y = false)
{
	copyStretchSpriteToScreen(_memory->spriteSheetData, sx, sy, sw, sh, dx, dy, dw, dh, flip_x, flip_y);
}

bool Graphics::fget(uint8_t n, uint8_t f){
	return (this->fget(n) >> f) & 1;
}

uint8_t Graphics::fget(uint8_t n){
	return _memory->spriteFlags[n];
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
	_memory->spriteFlags[n] = v;
}

uint8_t Graphics::sget(uint8_t x, uint8_t y){
	return getPixelNibble(x, y, _memory->spriteSheetData);
}

void Graphics::sset(uint8_t x, uint8_t y, uint8_t c){
	setPixelNibble(x, y, c, _memory->spriteSheetData);
}

void Graphics::camera() {
	this->camera(0, 0);
}

void Graphics::camera(int x, int y) {
	_memory->drawState.camera_x = x;
	_memory->drawState.camera_y = y;
}

void Graphics::clip() {
	this->clip(0, 0, 128, 128);
}

void Graphics::clip(int x, int y, int w, int h) {
	int xe = x + w;
	int ye = y + h;
	_memory->drawState.clip_xb = clampCoordToScreenDims(x);
	_memory->drawState.clip_yb = clampCoordToScreenDims(y);
	_memory->drawState.clip_xe = clampCoordToScreenDims(xe);
	_memory->drawState.clip_ye = clampCoordToScreenDims(ye);
}


//map methods heavily based on tac08 implementation
uint8_t Graphics::mget(int celx, int cely){
	if (cely < 32) {
		return _memory->mapData[cely * 128 + celx];
	}
	else if (cely < 64){
		return _memory->spriteSheetData[cely* 128 + celx];
	}

	return 0;
}

void Graphics::mset(int celx, int cely, uint8_t snum){
	if (cely < 32) {
		_memory->mapData[cely * 128 + celx] = snum;
	}
	else if (cely < 64){
		_memory->spriteSheetData[cely* 128 + celx] = snum;
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
		_memory->drawState.drawPaletteMap[c] = c;
		_memory->drawState.screenPaletteMap[c] = c;
	}

	this->palt();
}

void Graphics::pal(uint8_t c0, uint8_t c1, uint8_t p){
	//0-15 alowed
	c0 &= 0x0f;
	if (p == 0) {
		//for draw palette we have to preserve the transparency bit
		_memory->drawState.drawPaletteMap[c0] = (_memory->drawState.drawPaletteMap[c0] & 0x10) | (c1 & 0xf);
	} else if (p == 1) {
		//0-15, or 127-143 allowed
		c1 &= 0x8f;
		_memory->drawState.screenPaletteMap[c0] = c1;
	}
}

void Graphics::palt() {
	_memory->drawState.drawPaletteMap[0] |= 1UL << 4;
	for (uint8_t c = 1; c < 16; c++) {
		_memory->drawState.drawPaletteMap[c] &= ~(1UL << 4);
	}
}

void Graphics::palt(uint8_t c, bool t){
	c = c & 15;
	if (t) {
		_memory->drawState.drawPaletteMap[c] |= 1UL << 4;
	}
	else {
		_memory->drawState.drawPaletteMap[c] &= ~(1UL << 4);
	}
}


void Graphics::cursor() {
	this->cursor(0, 0);
}

void Graphics::cursor(int x, int y) {
	_memory->drawState.text_x = x;
	_memory->drawState.text_y = y;
}

void Graphics::cursor(int x, int y, uint8_t col) {
	color(col);

	this->cursor(x, y);
}


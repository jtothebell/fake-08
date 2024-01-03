#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>
#include <vector>
#include <tuple>
using namespace std;

#include "graphics.h"
#include "hostVmShared.h"
#include "nibblehelpers.h"
#include "mathhelpers.h"
#include "fontdata.h"

#include "stringToDataHelpers.h"

#include "logger.h"

#include <fix32.h>
using namespace z8;


//call initialize to make sure defaults are correct
Graphics::Graphics(std::string fontdata, PicoRam* memory) {
	_memory = memory;
	
	copy_string_to_sprite_memory(fontSpriteData, fontdata);

	//set default clip
	clip();
	pal();
	color();
}


uint8_t* Graphics::GetP8FrameBuffer(){
	return _memory->hwState.screenDataMemMapping == 0 
		? _memory->spriteSheetData 
		: _memory->screenBuffer;
}

uint8_t* Graphics::GetP8SpriteSheetBuffer(){
	return _memory->hwState.spriteSheetMemMapping == 0x60
		? _memory->screenBuffer 
		: _memory->spriteSheetData;
}

uint8_t* Graphics::GetScreenPaletteMap(){
	return _memory->drawState.screenPaletteMap;
}

//start helper methods
//based on tac08 implementation of blitter()
void Graphics::copySpriteToScreen(
	uint8_t* spritebuffer,
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

	auto &drawState = _memory->drawState;
	auto &hwState = _memory->hwState;
	uint8_t *screenBuffer = GetP8FrameBuffer();
	
	const uint8_t writeMask = hwState.colorBitmask & 15;
	const uint8_t readMask = hwState.colorBitmask >> 4;

	scr_x -= drawState.camera_x;
	scr_y -= drawState.camera_y;

	// left clip
	if (scr_x < drawState.clip_xb) {
		int nclip = drawState.clip_xb - scr_x;
		scr_x = drawState.clip_xb;
		scr_w -= nclip;
		if (!flip_x) {
			spr_x += nclip;
		} else {
			spr_w -= nclip;
		}
	}

	// right clip
	if (scr_x + scr_w > drawState.clip_xe) {
		int nclip = (scr_x + scr_w) - drawState.clip_xe;
		scr_w -= nclip;
	}

	// top clip
	if (scr_y < drawState.clip_yb) {
		int nclip = drawState.clip_yb - scr_y;
		scr_y = drawState.clip_yb;
		scr_h -= nclip;
		if (!flip_y) {
			spr_y += nclip;
		} else {
			spr_h -= nclip;
		}
	}

	// bottom clip
	if (scr_y + scr_h > drawState.clip_ye) {
		int nclip = (scr_y + scr_h) - drawState.clip_ye;
		scr_h -= nclip;
	}
	uint8_t lastScreenBuffByte = 0;
	int lastScreenBuffIdx = -1;

	bool startWithHalf = false;
	//starting with odd pixel
	if (BITMASK(0) & spr_x) {
		startWithHalf = true;
	}

	//since we get 2 pixels at a time, don't go through this whole loop every pixel
	for (int y = 0; y < scr_h; y++) {
		int x = 0;
		while (x < scr_w) {
			int abs_spr_x = spr_x + (flip_x ? spr_w - (x + 1) : x);
			int abs_spr_y = spr_y + (flip_y ? spr_h - (y + 1) : y);
			if (!IS_VALID_SPR_IDX(abs_spr_x, abs_spr_y)){
				++x;
				continue;
			}
			uint8_t bothPix = spritebuffer[COMBINED_IDX(abs_spr_x, abs_spr_y)];

			//uint8_t c = (BITMASK(0) & abs_spr_x)== 0 
			//			? bothPix & 0x0f //just first 4 bits
			//			: bothPix >> 4;  //just last 4 bits
			uint8_t lc = bothPix & 0x0f;
			uint8_t rc = bothPix >> 4;
			
			const int finaly = scr_y + y;
			int finalx = scr_x + (flip_x ? x + 1 : x);
			
			if (x > 0 || !startWithHalf){
				if (!(drawState.drawPaletteMap[lc] >> 4)){
					lc = drawState.drawPaletteMap[lc] & 0x0f;

					int screenPixelIdx = COMBINED_IDX(finalx, finaly);
					if (lastScreenBuffIdx != screenPixelIdx) {
						lastScreenBuffByte = screenBuffer[screenPixelIdx];
						lastScreenBuffIdx = screenPixelIdx;
					}

					uint8_t source = (BITMASK(0) & finalx) == 0 
						? lastScreenBuffByte & 0x0f //just first 4 bits
						: lastScreenBuffByte >> 4;

					lc = (source & ~writeMask) | (lc & writeMask & readMask);

					setPixelNibble(finalx, finaly, lc, screenBuffer);
				}
				++x;
				if (flip_x){
					--finalx;
				}
				else {
					++finalx;
				}
			}

			if (x < scr_w) {
				if (!(drawState.drawPaletteMap[rc] >> 4)){
					rc = drawState.drawPaletteMap[rc] & 0x0f;

					int screenPixelIdx = COMBINED_IDX(finalx, finaly);
					if (lastScreenBuffIdx != screenPixelIdx) {
						lastScreenBuffByte = screenBuffer[screenPixelIdx];
						lastScreenBuffIdx = screenPixelIdx;
					}

					uint8_t source = (BITMASK(0) & finalx) == 0 
						? lastScreenBuffByte & 0x0f //just first 4 bits
						: lastScreenBuffByte >> 4;

					rc = (source & ~writeMask) | (rc & writeMask & readMask);

					setPixelNibble(finalx, finaly, rc, screenBuffer);
				}

				//we did two pixels so do an extra increment
				++x;
			}
			
		}

	}
}

//based on tac08 implementation of stretch_blitter()
//uses ints so we can shift bits and do integer division instead of floating point
void Graphics::copyStretchSpriteToScreen(
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
	//skipStretchPx is currently only used for drawing stripey mode text,
	//so it is only used when drawing in non-flipped mode
	bool skipStretchPx)
{
	if (scr_w == 0 || scr_h == 0)
		return;

	if (spr_h == scr_h && spr_w == scr_w && !flip_x) {
		// use faster non stretch blitter if sprite is not stretched 
		//(or flipped horizontally - but skipping for that is just a hacky fix)
		copySpriteToScreen(spritebuffer, scr_x, scr_y, spr_x, spr_y, scr_w, scr_h, flip_x, flip_y);
		return;
	}

	auto &drawState = _memory->drawState;
	auto &hwState = _memory->hwState;
	uint8_t *screenBuffer = GetP8FrameBuffer();

	const uint8_t writeMask = hwState.colorBitmask & 15;
	const uint8_t readMask = hwState.colorBitmask >> 4;

	scr_x -= drawState.camera_x;
	scr_y -= drawState.camera_y;

	if (scr_w < 0){
		flip_x = !flip_x;
		scr_w = -scr_w;
		scr_x -= scr_w;
	}
	if (scr_h < 0) {
		flip_y = !flip_y;
		scr_h = -scr_h;
		scr_y -= scr_h;
	}

	//shift bits to avoid floating point math
	spr_x = spr_x << 16;
	spr_y = spr_y << 16;
	spr_w = spr_w << 16;
	spr_h = spr_h << 16;

	int dx = spr_w / scr_w;
	int dy = spr_h / scr_h;

	// left clip
	if (scr_x < drawState.clip_xb) {
		int nclip = drawState.clip_xb - scr_x;
		scr_x = drawState.clip_xb;
		scr_w -= nclip;
		if (!flip_x) {
			spr_x += nclip * dx;
		} else {
			spr_w -= nclip * dx;
		}
	}

	// right clip
	if (scr_x + scr_w > drawState.clip_xe) {
		int nclip = (scr_x + scr_w) - drawState.clip_xe;
		scr_w -= nclip;
	}

	// top clip
	if (scr_y < drawState.clip_yb) {
		int nclip = drawState.clip_yb - scr_y;
		scr_y = drawState.clip_yb;
		scr_h -= nclip;
		if (!flip_y) {
			spr_y += nclip * dy;
		} else {
			spr_h -= nclip * dy;
		}
	}

	// bottom clip
	if (scr_y + scr_h > drawState.clip_ye) {
		int nclip = (scr_y + scr_h) - drawState.clip_ye;
		scr_h -= nclip;
	}

	if (flip_y) {
		spr_y += spr_h - 1 * dy;
		dy = -dy;
	}

	int prevSprX = -1;
	int prevSprY = -1;

	//ugly duplication but see if inlining helps
	if (hwState.colorBitmask == 0xff){
		for (int y = 0; y < scr_h; y++) {
			int sprY = ((spr_y + y * dy) >> 16);
			if (sprY > 127) {
				continue;
			}
			uint8_t* spr = spritebuffer + (sprY * 64);

			if (skipStretchPx && prevSprY == sprY){
				continue;
			}
			
			prevSprY = sprY;

			if (!flip_x) {
				for (int x = 0; x < scr_w; x++) {
					int shiftedPixIndex = (spr_x + x * dx) >> 16;
					if (shiftedPixIndex > 127) {
						continue;
					}
					if (skipStretchPx && prevSprX == shiftedPixIndex){
						continue;
					}
					prevSprX = shiftedPixIndex;
					int preShiftedCombinedPixIndex = (shiftedPixIndex / 2);
					uint8_t bothPix = spr[preShiftedCombinedPixIndex];

					uint8_t c = shiftedPixIndex % 2 == 0 
						? bothPix & 0x0f //just first 4 bits
						: bothPix >> 4;  //just last 4 bits

					if (_memory->drawState.drawPaletteMap[c] >> 4){
						continue;
					}
					c = drawState.drawPaletteMap[c] & 0x0f;
					setPixelNibble(scr_x + x, scr_y + y, c, screenBuffer);
				}
			} else {
				for (int x = 0; x < scr_w; x++) {
					int pixIndex = (spr_x + spr_w - (x + 1) * dx);
					int shiftedPixIndex = pixIndex >> 16;
					if (shiftedPixIndex > 127) {
						continue;
					}
					int combinedPixIdx = shiftedPixIndex / 2;
					uint8_t bothPix = spr[combinedPixIdx];

					uint8_t c = (pixIndex >> 16) % 2 == 0 
						? bothPix & 0x0f //just first 4 bits
						: bothPix >> 4;  //just last 4 bits
					
					if (_memory->drawState.drawPaletteMap[c] >> 4){
						continue;
					}
					c = drawState.drawPaletteMap[c] & 0x0f;
					setPixelNibble(scr_x + x, scr_y + y, c, screenBuffer);
				}
			}
		}
	}
	else {
		for (int y = 0; y < scr_h; y++) {
			int sprY = ((spr_y + y * dy) >> 16);
			if (sprY > 127) {
				continue;
			}
			uint8_t* spr = spritebuffer + (sprY) * 64;

			if (!flip_x) {
				for (int x = 0; x < scr_w; x++) {
					int shiftedPixIndex = (spr_x + x * dx) >> 16;
					if (shiftedPixIndex > 127) {
						continue;
					}
					int combinedPixIdx = (shiftedPixIndex / 2);
					uint8_t bothPix = spr[combinedPixIdx];

					uint8_t c = shiftedPixIndex % 2 == 0 
						? bothPix & 0x0f //just first 4 bits
						: bothPix >> 4;  //just last 4 bits

					if (_memory->drawState.drawPaletteMap[c] >> 4){
						continue;
					}
					c = drawState.drawPaletteMap[c] & 0x0f;

					const int finalx = scr_x + x;
					const int finaly = scr_y + y;

					uint8_t source = (BITMASK(0) & finalx) == 0 
							? screenBuffer[COMBINED_IDX(finalx, finaly)] & 0x0f //just first 4 bits
							: screenBuffer[COMBINED_IDX(finalx, finaly)] >> 4;

					c = (source & ~writeMask) | (c & writeMask & readMask);

					setPixelNibble(finalx, finaly, c, screenBuffer);
				}
			} else {
				for (int x = 0; x < scr_w; x++) {
					int pixIndex = (spr_x + spr_w - (x + 1) * dx);
					int shiftedPixIndex = pixIndex >> 16;
					if (shiftedPixIndex > 127) {
						continue;
					}
					int combinedPixIdx = (shiftedPixIndex / 2);
					uint8_t bothPix = spr[combinedPixIdx];

					uint8_t c = (pixIndex >> 16) % 2 == 0 
						? bothPix & 0x0f //just first 4 bits
						: bothPix >> 4;  //just last 4 bits

					if (_memory->drawState.drawPaletteMap[c] >> 4){
						continue;
					}
					c = drawState.drawPaletteMap[c] & 0x0f;
					
					const int finalx = scr_x + x;
					const int finaly = scr_y + y;
					
					uint8_t source = (BITMASK(0) & finalx) == 0 
							? screenBuffer[COMBINED_IDX(finalx, finaly)] & 0x0f //just first 4 bits
							: screenBuffer[COMBINED_IDX(finalx, finaly)] >> 4;

					c = (source & ~writeMask) | (c & writeMask & readMask);

					setPixelNibble(finalx, finaly, c, screenBuffer);
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
	return (_memory->drawState.drawPaletteMap[color] >> 4) > 0; //upper bits indicate transparency
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
	return clamp(val, 0, 128);
}

int Graphics::clampXCoordToClip(int x) {
	return clamp(
		x,
		(int)_memory->drawState.clip_xb,
		(int)_memory->drawState.clip_xe - 1);
}

int Graphics::clampYCoordToClip(int y) {
	return clamp(
		y,
		(int)_memory->drawState.clip_yb,
		(int)_memory->drawState.clip_ye - 1);
}


void Graphics::_safeSetPixelFromPen(int x, int y) {
	if (isWithinClip(x, y)){
		_setPixelFromPen(x, y);
	}
}

void Graphics::_setPixelFromSprite(int x, int y, uint8_t col) {
	x = x & 127;
	y = y & 127;

	auto &drawState = _memory->drawState;
	auto &hwState = _memory->hwState;
	uint8_t *screenBuffer = GetP8FrameBuffer();

	//col = getDrawPalMappedColor(col);
	col = drawState.drawPaletteMap[col & 0x0f] & 0x0f; 

	if (hwState.colorBitmask != 0xff) {
		//from pico 8 wiki:
		//dst_color = (dst_color & ~write_mask) | (src_color & write_mask & read_mask)
		uint8_t writeMask = hwState.colorBitmask & 15;
		uint8_t readMask = hwState.colorBitmask >> 4;
		//uint8_t source = pget(x, y);
		uint8_t source = (BITMASK(0) & x) == 0 
				? screenBuffer[COMBINED_IDX(x, y)] & 0x0f //just first 4 bits
				: screenBuffer[COMBINED_IDX(x, y)] >> 4;
		col = (source & ~writeMask) | (col & writeMask & readMask);
	}

	setPixelNibble(x, y, col, screenBuffer);
}

void Graphics::_setPixelFromPen(int x, int y) {
	x = x & 127;
	y = y & 127;

	auto &drawState = _memory->drawState;
	auto &hwState = _memory->hwState;
	uint8_t *screenBuffer = GetP8FrameBuffer();

	uint8_t col = drawState.color;

	uint8_t col0 = col & 0x0f;
	uint8_t col1 = (col & 0xf0) >> 4;

	uint8_t finalC = col0;


	//uint8_t gridX = x % 4;
	//uint8_t gridY = y % 4;
	//uint8_t bitPlace = gridY * 4 + gridX;
	uint8_t bitPlace = 15 - ((x & 3) + 4 * (y & 3));

	uint16_t fillp = ((uint16_t)drawState.fillPattern[1] << 8) + drawState.fillPattern[0];
	bool altColor = (fillp >> bitPlace) & 0x1;
	if (altColor) {
		if (drawState.fillPatternTransparencyBit & 1){
			return;
		}

		finalC = col1;
	}

	//finalC = getDrawPalMappedColor(finalC);
	finalC = drawState.drawPaletteMap[finalC & 0x0f] & 0x0f; 

	//from pico 8 wiki:
	//dst_color = (dst_color & ~write_mask) | (src_color & write_mask & read_mask)
	if (hwState.colorBitmask != 0xff){
		uint8_t writeMask = hwState.colorBitmask & 15;
		uint8_t readMask = hwState.colorBitmask >> 4;

		//camera should already be applied, and x and y should be safe coords by now, so don't use pget
		//uint8_t source = pget(x, y);
		uint8_t source = (BITMASK(0) & x) == 0 
			? screenBuffer[COMBINED_IDX(x, y)] & 0x0f //just first 4 bits
			: screenBuffer[COMBINED_IDX(x, y)] >> 4;

		finalC = (source & ~writeMask) | (finalC & writeMask & readMask);
	}

	setPixelNibble(x, y, finalC, screenBuffer);
}
//end helper methods

void Graphics::cls() {
	this->cls(0);
}

void Graphics::cls(uint8_t color) {
	color = color & 15;
	uint8_t val = color << 4 | color;
	memset(GetP8FrameBuffer(), val, sizeof(_memory->screenBuffer));

	_memory->drawState.text_x = 0;
	_memory->drawState.text_y = 0;

	_memory->drawState.clip_xb = 0;
	_memory->drawState.clip_yb = 0;
	_memory->drawState.clip_xe = 128;
	_memory->drawState.clip_ye = 128;
}

void Graphics::pset(int x, int y){
	this->pset(x, y, _memory->drawState.color);
}

void Graphics::pset(int x, int y, uint8_t col){
	color(col);

	applyCameraToPoint(&x, &y);

	if (isWithinClip(x, y)){
		_setPixelFromPen(x, y);
	}
}

uint8_t Graphics::pget(int x, int y){
	applyCameraToPoint(&x, &y);

	if (isOnScreen(x, y)){
		return getPixelNibble(x, y, GetP8FrameBuffer());
	}

	return 0;
}

uint8_t Graphics::color(){
	return color(6);
}

uint8_t Graphics::color(uint8_t col){
	uint8_t prev = _memory->drawState.color;

	_memory->drawState.color = col;

	return prev;
}

void Graphics::line () {
	//just invalidate line state
	_memory->drawState.line_x = 0;
	_memory->drawState.line_y = 0;
	_memory->drawState.lineInvalid = 1;
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

void Graphics::_private_h_line(int x1, int x2, int y){
	auto &drawState = _memory->drawState;
	auto &hwState = _memory->hwState;
	uint8_t * screenBuffer = GetP8FrameBuffer();

	if (!(y >= drawState.clip_yb && y < drawState.clip_ye)) {
		return;
	}

	if ((x1 < drawState.clip_xb && x2 < drawState.clip_xb) ||
		(x1 > drawState.clip_xe && x2 > drawState.clip_xe)) {
			return;
	}

	int maxx = clamp(
		std::max(x1, x2),
		(int)drawState.clip_xb,
		(int)drawState.clip_xe - 1);

	int minx = clamp(
		std::min(x1, x2),
		(int)drawState.clip_xb,
		(int)drawState.clip_xe - 1);

	bool canmemset = hwState.colorBitmask == 0xff && 
		drawState.fillPattern[0] == 0 && 
		drawState.fillPattern[1] == 0 &&
		maxx - minx > 1;

	if (canmemset) {
		//zepto 8 adapted otimized line draw with memset
		uint8_t *p = screenBuffer + (y*64);
        uint8_t color = getDrawPalMappedColor(drawState.color);

        if (minx & 1)
        {
			
            p[minx / 2] = (p[minx / 2] & 0x0f) | (color << 4);
            ++minx;
        }

        if ((maxx & 1) == 0)
        {
			
            p[maxx / 2] = (p[maxx / 2] & 0xf0) | color;
            --maxx;
        }

        memset(p + minx / 2, color * 0x11, (maxx - minx + 1) / 2);
	}
	else {
		for (int x = minx; x <= maxx; x++){
			_setPixelFromPen(x, y);
		}
	}
}

void Graphics::_private_v_line (int y1, int y2, int x){
	auto &drawState = _memory->drawState;
	auto &hwState = _memory->hwState;
	uint8_t * screenBuffer = GetP8FrameBuffer();

	if (!(x >= drawState.clip_xb && x < drawState.clip_xe)) {
		return;
	}

	if ((y1 < drawState.clip_yb && y2 < drawState.clip_yb) ||
		(y1 > drawState.clip_ye && y2 > drawState.clip_ye)) {
			return;
	}

	int maxy = clamp(
		std::max(y1, y2),
		(int)drawState.clip_yb,
		(int)drawState.clip_ye - 1);

	int miny = clamp(
		std::min(y1, y2),
		(int)drawState.clip_yb,
		(int)drawState.clip_ye - 1);

	bool skipPen = hwState.colorBitmask == 0xff && 
		drawState.fillPattern[0] == 0 && 
		drawState.fillPattern[1] == 0;

	if (skipPen) {
		uint8_t color = getDrawPalMappedColor(drawState.color);
		uint8_t mask = (x & 1) ? 0x0f : 0xf0;
		uint8_t nibble = (x & 1) ? color << 4 : color;

		for (int16_t y = miny; y <= maxy; ++y)
        {
			int pixIdx = COMBINED_IDX(x, y);
            auto &data = screenBuffer[pixIdx];
            data = (data & mask) | nibble;
        }
	}
	else {
		for (int y = miny; y <= maxy; y++){
			_setPixelFromPen(x, y);
		}
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
		_private_v_line(y0, y1, x0);
	} 
	else if (y0 == y1) {
		_private_h_line(x0, x1, y0);
	}
	else {
		//tac08 line impl for diagonals (this should work for horizontal and vertical as well,
		//but it has more branching)
		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = dx + dy, e2; /* error value e_xy */

		for (;;) { /* loop */
			_safeSetPixelFromPen(x0, y0);
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

fix32 getTlineMask(uint8_t dim){
	if (dim == 0) {
		return fix32::frombits(0xffffff);
	}

	return fix32(dim) - fix32::frombits(1);
}

//ported from zepto 8 impl
void Graphics::tline(int x0, int y0, int x1, int y1, fix32 mx, fix32 my, fix32 mdx, fix32 mdy){
	applyCameraToPoint(&x0, &y0);
	applyCameraToPoint(&x1, &y1);

	//determine whether x or y coordinates need to get incremented
	bool xDifGreater = std::abs(x1 - x0) >= std::abs(y1 - y0);
	int dx = xDifGreater ? x0 <= x1 ? 1 : -1 : 0;
    int dy = xDifGreater ? 0 : y0 <= y1 ? 1 : -1;

	bool vertical = x0 == x1;

	int x = clampXCoordToClip(x0);
	int xend = clampXCoordToClip(x1);
	int y = clampYCoordToClip(y0);
	int yend = clampYCoordToClip(y1);

	auto &ds = _memory->drawState;

	// Retrieve masks for wrap-around and subtract 0x0.0001
	fix32 xmask = getTlineMask(ds.tlineMapWidth);
	fix32 ymask = getTlineMask(ds.tlineMapHeight);

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
        int sx = (ds.tlineMapXOffset + int(mx & xmask));
        int sy = (ds.tlineMapYOffset + int(my & ymask));
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

            if (!isColorTransparent(col) && isWithinClip(x, y)) {
                _setPixelFromSprite(x, y, col);
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
		_safeSetPixelFromPen(ox + x, oy + y);
		_safeSetPixelFromPen(ox + y, oy + x);
		_safeSetPixelFromPen(ox - x, oy + y);
		_safeSetPixelFromPen(ox - y, oy + x);

		_safeSetPixelFromPen(ox - x, oy - y);
		_safeSetPixelFromPen(ox - y, oy - x);
		_safeSetPixelFromPen(ox + x, oy - y);
		_safeSetPixelFromPen(ox + y, oy - x);

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
		_safeSetPixelFromPen(ox, oy);
	}
	else if (r == 1) {
		_safeSetPixelFromPen(ox, oy - 1);
		_private_h_line(ox-1, ox+1, oy);
		_safeSetPixelFromPen(ox, oy + 1);
	}
	else if (r > 0) {
		int x = -r, y = 0, err = 2 - 2 * r;
		do {
			_private_h_line(ox - x, ox + x, oy + y);
			_private_h_line(ox - x, ox + x, oy - y);
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

	_safeSetPixelFromPen(xc, yc+yr);
    _safeSetPixelFromPen(xc, yc-yr);

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

        if (xa >= ya) {
        	break;
		}


        _safeSetPixelFromPen(xc+wx, yc-wy);
        _safeSetPixelFromPen(xc-wx, yc-wy);
        _safeSetPixelFromPen(xc+wx, yc+wy);
        _safeSetPixelFromPen(xc-wx, yc+wy);
    }

    _safeSetPixelFromPen(xc+xr, yc);
    _safeSetPixelFromPen(xc-xr, yc);

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

        if (ya > xa) {
        	break;
		}
		if (ya == 0 && xa == 0) {
			break;
		}

        _safeSetPixelFromPen(xc+wx, yc-wy);
        _safeSetPixelFromPen(xc-wx, yc-wy);
        _safeSetPixelFromPen(xc+wx, yc+wy);
        _safeSetPixelFromPen(xc-wx, yc+wy);
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

	_private_v_line(yc+yr, yc-yr, xc);

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

        if (xa >= ya) {
        	break;
		}

		_private_h_line(xc+wx, xc-wx, yc-wy);
		_private_h_line(xc+wx, xc-wx, yc+wy);
    }

	_private_h_line(xc+xr, xc-xr, yc);

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

        if (ya > xa) {
        	break;
		}
		if (ya == 0 && xa == 0) {
			break;
		}

		_private_h_line(xc+wx, xc-wx, yc-wy);
		_private_h_line(xc+wx, xc-wx, yc+wy);
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
	_memory->drawState.color = col;

	//applyCameraToPoint(&x1, &y1);
	x1 -= _memory->drawState.camera_x;
	y1 -= _memory->drawState.camera_y;
	//applyCameraToPoint(&x2, &y2);
	x2 -= _memory->drawState.camera_x;
	y2 -= _memory->drawState.camera_y;

	int temp;

	if (x1 > x2) {
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	_private_h_line(x1, x2, y1);
	_private_h_line(x1, x2, y2);

	_private_v_line(y1, y2, x1);
	_private_v_line(y1, y2, x2);
}

void Graphics::rectfill(int x1, int y1, int x2, int y2) {
	this->rectfill(x1, y1, x2, y2, _memory->drawState.color);
}

void Graphics::rectfill(int x1, int y1, int x2, int y2, uint8_t col) {
	auto &drawState = _memory->drawState;

	drawState.color = col;

	//applyCameraToPoint(&x1, &y1);
	x1 -= drawState.camera_x;
	y1 -= drawState.camera_y;
	//applyCameraToPoint(&x2, &y2);
	x2 -= drawState.camera_x;
	y2 -= drawState.camera_y;

	int temp;

	if (x1 > x2) {
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	}

	for (int y = y1; y <= y2; y++) {
		_private_h_line(x1, x2, y);
	}
}

fix32 Graphics::fillp(fix32 pat) {
	int32_t prev = (_memory->drawState.fillPattern[0] << 16)
                 | (_memory->drawState.fillPattern[1] << 24)
                 | (_memory->drawState.fillPatternTransparencyBit << 8);

	uint8_t patByte0 = pat.bits() >> 16;
	uint8_t patByte1 = pat.bits() >> 24;

	uint8_t patTranspByte = pat.bits() >> 15;

	_memory->drawState.fillPattern[0] = patByte0;
	_memory->drawState.fillPattern[1] = patByte1;

	_memory->drawState.fillPatternTransparencyBit = patTranspByte & 1;

	return z8::fix32::frombits(prev);
}


int Graphics::drawCharacter(
	uint8_t ch,
	int x,
	int y,
	uint8_t fgColor,
	uint8_t bgColor,
	uint8_t printMode,
	int forceCharWidth,
	int forceCharHeight) {
	int extraCharWidth = 0;

	bool useCustomFont = (printMode & PRINT_MODE_CUSTOM_FONT) == PRINT_MODE_CUSTOM_FONT;
	int defaultCharWidth = useCustomFont ? _memory->data[0x5600] : 4;
	int defaultWideCharWidth = useCustomFont ? _memory->data[0x5601] : 8;
	int defaultCharHeight = useCustomFont ? _memory->data[0x5602] : 5;

	if (ch > 0x0f) {
		uint8_t charWidth = 
			ch < 0x80
			? forceCharWidth > -1 && forceCharWidth < 4 ? forceCharWidth : defaultCharWidth
			: forceCharWidth > -1 && forceCharWidth < 4 ? (forceCharWidth + 4) : defaultWideCharWidth;

		if (ch >= 0x80) {
			extraCharWidth = defaultWideCharWidth - defaultCharWidth;
		}

		uint8_t charHeight = forceCharHeight > -1 && forceCharHeight < 5 ? forceCharHeight : defaultCharHeight;
		
		auto result = drawCharacterFromBytes(
			useCustomFont ? &(_memory->data[0x5600 + ch*8]) : &(defaultFontBinaryData[ch*8]),
			x,
			y,
			fgColor,
			bgColor,
			printMode,
			charWidth,
			charHeight
		);
		extraCharWidth += get<0>(result);
	}

	if ((printMode & PRINT_MODE_ON) == PRINT_MODE_ON){
		return 0;
	}

	return extraCharWidth;
}

std::tuple<int, int> Graphics::drawCharacterFromBytes(
	uint8_t chBytes[],
	int x,
	int y,
	uint8_t fgColor,
	uint8_t bgColor,
	uint8_t printMode,
	uint8_t charWidth,
	uint8_t charHeight) {
	
	applyCameraToPoint(&x, &y);
	uint8_t *screenBuffer = GetP8FrameBuffer();
	
	int extraCharWidth = 0;
	int extraCharHeight = 0;
	int wFactor = 1;
	int hFactor = 1;
	bool evenPxOnly = false;
	bool invertColors = false;
	bool solidBg = false;

	if ((printMode & PRINT_MODE_ON) == PRINT_MODE_ON){
		if ((printMode & PRINT_MODE_WIDE) == PRINT_MODE_WIDE) {
			wFactor = 2;
			extraCharWidth = charWidth;
		}
		if((printMode & PRINT_MODE_TALL) == PRINT_MODE_TALL) {
			hFactor = 2;
			extraCharHeight = charHeight;
		}
		if((printMode & PRINT_MODE_STRIPEY) == PRINT_MODE_STRIPEY) {
			//draw every other pixel-- also kinda broken on pico 8 0.2.4 
			evenPxOnly = true;
		}
		if((printMode & PRINT_MODE_INVERTED) == PRINT_MODE_INVERTED) {
			invertColors = true;
		}
		if((printMode & PRINT_MODE_SOLID_BG) == PRINT_MODE_SOLID_BG) {
			solidBg = true;
		}
	}

	//possible todo: check perf if this is better than doing it in the other loop (m)
	// if (bgColor != 0) {
	// 	uint8_t prevPenColor = _memory->drawState.color;
	// 	rectfill(x-1, y-1, x + charWidth*wFactor - 1, y + charHeight*hFactor - 1, bgColor);
	// 	_memory->drawState.color = prevPenColor;
	// }
	fgColor &= 0x0f;
	bgColor &= 0x0f;

	for (int relDestY = 0; relDestY < charHeight * hFactor; relDestY++) {
		for(int relDestX = 0; relDestX < charWidth * wFactor; relDestX++) {

			bool on = BITMASK(relDestX / wFactor) & chBytes[relDestY / hFactor];
			on &= hFactor == 1 || !evenPxOnly || (relDestY % 2 == 0);
			on &= wFactor == 1 || !evenPxOnly || (relDestX % 2 == 0);

			int absDestX = x + relDestX;
			int absDestY = y + relDestY;

			if (isWithinClip(absDestX, absDestY)) {
				if (invertColors) {
					on = !on;
				}
				
				if (on) {
					setPixelNibble(absDestX, absDestY, fgColor, screenBuffer);			
				}
				if(!on && (solidBg || bgColor > 0)) {
					setPixelNibble(absDestX, absDestY, bgColor, screenBuffer);
				}
			}
		}
	}

	std::tuple<int, int> retVal (extraCharWidth, extraCharHeight);

	return retVal;
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
	copySpriteToScreen(GetP8SpriteSheetBuffer(), x, y, spr_x, spr_y, spr_w, spr_h, flip_x, flip_y);
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
	copyStretchSpriteToScreen(GetP8SpriteSheetBuffer(), sx, sy, sw, sh, dx, dy, dw, dh, flip_x, flip_y, false);
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
	if (IS_VALID_SPR_IDX(x, y)) {
		return getPixelNibble(x, y, GetP8SpriteSheetBuffer());
	}
	return 0;
}

void Graphics::sset(uint8_t x, uint8_t y, uint8_t c){
	if (IS_VALID_SPR_IDX(x, y)) {
		setPixelNibble(x, y, c, GetP8SpriteSheetBuffer());
	}
	return;
}

std::tuple<int16_t, int16_t> Graphics::camera() {
	return this->camera(0, 0);
}

std::tuple<int16_t, int16_t> Graphics::camera(int16_t x, int16_t y) {
	std::tuple<int16_t, int16_t> prev (_memory->drawState.camera_x, _memory->drawState.camera_y);

	_memory->drawState.camera_x = x;
	_memory->drawState.camera_y = y;

	return prev;
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> Graphics::clip() {
	return this->clip(0, 0, 128, 128);
}

std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> Graphics::clip(int x, int y, int w, int h) {
	auto prev = std::make_tuple(
		_memory->drawState.clip_xb,
		_memory->drawState.clip_xe,
		_memory->drawState.clip_yb,
		_memory->drawState.clip_ye 
	);

	int xe = x + w;
	int ye = y + h;
	_memory->drawState.clip_xb = clampCoordToScreenDims(x);
	_memory->drawState.clip_yb = clampCoordToScreenDims(y);
	_memory->drawState.clip_xe = clampCoordToScreenDims(xe);
	_memory->drawState.clip_ye = clampCoordToScreenDims(ye);

	return prev;
}


uint8_t Graphics::mget(int celx, int cely){
	const bool bigMap = _memory->hwState.mapMemMapping >= 0x80;
	const int bigMapLocation = _memory->hwState.mapMemMapping << 8;
	const int mapSize = bigMap 
		? 0x10000 - bigMapLocation
		: 8192;

	const int mapW = _memory->hwState.widthOfTheMap == 0 ? 256 : _memory->hwState.widthOfTheMap;
	const int mapH = mapSize / mapW;
	const int maxXIdx = mapW - 1;
	const int maxYIdx = mapH - 1;

	const int idx = cely * mapW + celx;

	if (celx < 0 || celx > maxXIdx || cely < 0 || cely > maxYIdx) {
        return 0;
	}
	
	if (bigMap){
		const int offset = 0x8000 - mapSize;
		return _memory->userData[offset + idx];
	}
	else {
		if (idx < 4096) {
			return _memory->mapData[idx];
		}
		else if (idx < 8192){
			return _memory->spriteSheetData[idx];
		}
	}

	return 0;
}

void Graphics::mset(int celx, int cely, uint8_t snum){
	const bool bigMap = _memory->hwState.mapMemMapping >= 0x80;
	const int bigMapLocation = _memory->hwState.mapMemMapping << 8;
	const int mapSize = bigMap 
		? 0x10000 - bigMapLocation
		: 8192;

	const int mapW = _memory->hwState.widthOfTheMap == 0 ? 256 : _memory->hwState.widthOfTheMap;
	const int mapH = mapSize / mapW;

	const int idx = cely * mapW + celx;

	if (celx < 0 || celx > mapW || cely < 0 || cely > mapH) {
        return;
	}

	if (bigMap){
		const int offset = 0x8000 - mapSize;
		_memory->userData[offset + idx] = snum;
	}
	else {
		if (idx < 4096)  {
			_memory->mapData[idx] = snum;
		}
		else if (idx < 8192) {
			_memory->spriteSheetData[idx] = snum;
		}
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

void Graphics::pal(uint8_t p) {
	for (uint8_t c = 0; c < 16; c++) {
		if (p == 0) {
			_memory->drawState.drawPaletteMap[c] = c;
		} else if (p == 1) {
			_memory->drawState.screenPaletteMap[c] = c;
		}
	}
}

uint8_t Graphics::pal(uint8_t c0, uint8_t c1, uint8_t p){
	//0-15 alowed
	c0 &= 0x0f;
	uint8_t prev = 0;
	if (p == 0) {
		//for draw palette we have to preserve the transparency bit
		prev = _memory->drawState.drawPaletteMap[c0] & 0xf;
		_memory->drawState.drawPaletteMap[c0] = (_memory->drawState.drawPaletteMap[c0] & 0x10) | (c1 & 0xf);
	} else if (p == 1) {
		//0-15, or 127-143 allowed
		prev = _memory->drawState.screenPaletteMap[c0] & 0xf;
		c1 &= 0x8f;
		_memory->drawState.screenPaletteMap[c0] = c1;
	}

	return prev;
}

void Graphics::palt() {
	_memory->drawState.drawPaletteMap[0] |= 1UL << 4;
	for (uint8_t c = 1; c < 16; c++) {
		_memory->drawState.drawPaletteMap[c] &= ~(1UL << 4);
	}
}

bool Graphics::palt(uint8_t c, bool t){
	c = c & 15;
	bool prev = _memory->drawState.drawPaletteMap[c] & 0xf0;
	if (t) {
		_memory->drawState.drawPaletteMap[c] |= 1UL << 4;
	}
	else {
		_memory->drawState.drawPaletteMap[c] &= ~(1UL << 4);
	}

	return prev;
}


std::tuple<uint8_t, uint8_t> Graphics::cursor() {
	return this->cursor(0, 0);
}

std::tuple<uint8_t, uint8_t> Graphics::cursor(int x, int y) {
	std::tuple<uint8_t, uint8_t> prev (_memory->drawState.text_x, _memory->drawState.text_y);

	_memory->drawState.text_x = x;
	_memory->drawState.text_y = y;

	return prev;
}

std::tuple<uint8_t, uint8_t> Graphics::cursor(int x, int y, uint8_t col) {
	color(col);

	return this->cursor(x, y);
}


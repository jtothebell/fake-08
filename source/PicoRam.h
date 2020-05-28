#include <string>

/*
0x0 	0x0fff 	Sprite sheet (0-127)
0x1000 	0x1fff 	Sprite sheet (128-255) / Map (rows 32-63) (shared)
0x2000 	0x2fff 	Map (rows 0-31)
0x3000 	0x30ff 	Sprite flags
0x3100 	0x31ff 	Music
0x3200 	0x42ff 	Sound effects
0x4300 	0x5dff 	General use (or work RAM)
0x5e00 	0x5eff 	Persistent cart data (64 numbers = 256 bytes)
0x5f00 	0x5f3f 	Draw state
0x5f40 	0x5f7f 	Hardware state
0x5f80 	0x5fff 	GPIO pins (128 bytes)
0x6000 	0x7fff 	Screen data (8k) 
*/

struct PicoRam
{
    uint8_t spriteSheetData[128 * 64];
	uint8_t mapData[128 * 32];
	uint8_t spriteFlags[256];

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

};
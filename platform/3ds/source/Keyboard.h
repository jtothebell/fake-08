#pragma once

#include <citro2d.h>

#include "../../source/host.h"


#define KEY_WIDTH 26
#define KEY_HEIGHT 36
#define	KEY_XOFFSET 4
#define KEY_YOFFSET 24

#define KEY_ROWS 6
#define KEY_COLUMNS 12

#define REPEAT_WAIT1 30
#define REPEAT_WAIT2 5

const std::string layout[KEY_ROWS][KEY_COLUMNS][2] = {
{{"1", "!" },{"2", "@" },{"3", "#" },{"4", "$" },{"5", "%" },{"6", "^" },{"7", "&" },{"8", "*" },{"9", "(" },{"0", ")" },{"-", "_" },{"=", "=" }},
{{"q", "Q" },{"w", "W" },{"e", "E" },{"r", "R" },{"t", "T" },{"y", "Y" },{"u", "U" },{"i", "I" },{"o", "O" },{"p", "P" },{"[", "{" },{"]", "}" }},
{{"a", "A" },{"s", "S" },{"d", "D" },{"f", "F" },{"g", "G" },{"h", "H" },{"j", "J" },{"k", "K" },{"l", "L" },{";", ":" },{"'", "\""},{"\r","\r"}},
{{"z", "Z" },{"x", "X" },{"c", "C" },{"v", "V" },{"b", "B" },{"n", "N "},{"m", "M" },{",", "<" },{".", ">" },{"/", "?" },{"\\","|" },{"\r","\r"}},
{{"!S","!S"},{"!S","!S"},{"!S","!S"},{" ", " " },{" ", " " },{" ", " " },{" ", " " },{" ", " " },{"!C","!C"},{"!C","!C"},{"!A","!A"},{"!A","!A"}},
{{"\t","\t"},{"\t","\t"},{"!E","!E"},{"!E","!E"},{"..",".."},{"..",".."},{"..",".."},{"..",".."},{"`" ,"~" },{"\b","\b"},{"\b","\b"},{"\b","\b"}}
};

const u32 darkenColor  = C2D_Color32(0x00, 0x00, 0x00, 0x80);

class Keyboard {
	
	bool enabled = false;
	C2D_SpriteSheet spritesheet;
	C2D_Sprite kbSprite;
	bool useEnableStretch = false;
	
	bool shift = false;
	bool ctrl = false;
	bool alt = false;
	bool symbol = false;
	
	bool touchDown = false;
	bool repeatingKey = false;
	int repeatTimer = 0;
	
	int tickSpeed = 1;
	
	int keyX = 0;
	int keyY = 0;
	std::string keyStr;
	std::string keyStrShift;
	
	StretchOption disableStretch;
	StretchOption enableStretch = AltScreenStretch;
	
public:
	Keyboard();
	void Init();
	void Toggle();
	void UpdateStretch(StretchOption& stretch);
	void UpdateTickSpeed(int fps);
	bool AllowMouse();
	void GetKey(bool& currKBDown, std::string& currKBKey, touchPosition& touch);
	void DarkenRect(int x, int y, int w);
	void Draw();
	void Cleanup();
};
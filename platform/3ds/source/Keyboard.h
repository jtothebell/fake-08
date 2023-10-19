#pragma once

#include <citro2d.h>

#include "../../source/host.h"


#define KEY_WIDTH 30
#define KEY_HEIGHT 36
#define	KEY_XOFFSET 0
#define KEY_YOFFSET 24

#define KEY_ROWS 6
#define KEY_COLUMNS 12

#define REPEAT_WAIT1 30
#define REPEAT_WAIT2 5

const std::string layout[KEY_ROWS][KEY_COLUMNS][2] = {
{{"1", "!" },{"2", "@" },{"3", "#" },{"4", "$" },{"5", "%" },{"6", "^" },{"7", "&" },{"8", "*" },{"9", "(" },{"0", ")" },{"-", "_" },{"=", "=" }},
{{"Q", "q" },{"W", "w" },{"E", "e" },{"R", "r" },{"T", "t" },{"Y", "y" },{"U", "u" },{"I", "i" },{"O", "o" },{"P", "p" },{"[", "{" },{"]", "}" }},
{{"A", "a" },{"S", "s" },{"D", "d" },{"F", "f" },{"G", "g" },{"H", "h" },{"J", "j" },{"K", "k" },{"L", "l" },{";", ":" },{"'", "\""},{"\r","\r"}},
{{"Z", "z" },{"X", "x" },{"C", "c" },{"V", "v" },{"B", "b" },{"N", "n "},{"M", "m" },{",", "<" },{".", ">" },{"/", "?" },{"\\","|" },{"\r","\r"}},
{{"!S","!S"},{"!S","!S"},{"!S","!S"},{" ", " " },{" ", " " },{" ", " " },{" ", " " },{" ", " " },{"!C","!C"},{"!C","!C"},{"!A","!A"},{"!A","!A"}},
{{"\t","\t"},{"\t","\t"},{"!E","!E"},{"!E","!E"},{"..",".."},{"..",".."},{"..",".."},{"..",".."},{"`" ,"~" },{"\b","\b"},{"\b","\b"},{"\b","\b"}}
};

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
	void Draw();
	void Cleanup();
};
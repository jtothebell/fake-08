#pragma once

#include <citro2d.h>

#include "../../source/host.h"

class Keyboard {
	
	bool enabled = false;
	C2D_SpriteSheet spritesheet;
	C2D_Sprite kbSprite;
	
	bool useEnableStretch = false;
	StretchOption disableStretch;
	StretchOption enableStretch = AltScreenStretch;
	
public:
	Keyboard();
	void Init();
	void Toggle();
	void UpdateStretch(StretchOption& stretch);
	void Draw();
	void Cleanup();
};
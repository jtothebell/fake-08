#include "Keyboard.h"

#include <citro2d.h>

#include "../../source/logger.h"
#include "../../source/host.h"
#include "../../source/emojiconversion.h"

Keyboard::Keyboard(){
	
};

void Keyboard::Init(){
	Logger_Write("Initializing keyboard\n");
	
	romfsInit();
	spritesheet = C2D_SpriteSheetLoad("romfs:/gfx/keyboard.t3x");
	C2D_SpriteFromSheet(&kbSprite, spritesheet, 0);
	C2D_SpriteSetPos(&kbSprite,0,0);
	
	enabled = false;
}

void Keyboard::Toggle(){
	enabled = !enabled;
	if(enabled){
		
		Logger_Write("KB Enabled\n");
	}
	else {
		Logger_Write("KB Disabled\n");
	}
}

void Keyboard::UpdateTickSpeed(int fps){
	
	tickSpeed = 60 / fps; //1 at 60, 2 at 30
}

void Keyboard::UpdateStretch(StretchOption& stretch) {
	if(enabled){
		if(!useEnableStretch){
			useEnableStretch = true;
			if(enableStretch == AltScreenStretch){ //first time keyboard has changed the stretch, match current stretch
				if(stretch == AltScreenPixelPerfect){
					enableStretch = PixelPerfect;
				}
				else if(stretch == AltScreenStretch){
					enableStretch = StretchToFit;
				}
				else {
					enableStretch = stretch;
				}
			}
			disableStretch = stretch;
			stretch = enableStretch;
			
		} 
		else {
			if(stretch == AltScreenPixelPerfect){
				stretch = PixelPerfect;
			}
			if(stretch == AltScreenStretch){
				stretch = StretchToFit;
			}
		}
	} 
	else {
		if(useEnableStretch){
			useEnableStretch = false;
			if(enableStretch != AltScreenStretch){
				enableStretch = stretch;
				stretch = disableStretch;
			}
		}
	}
}

bool Keyboard::AllowMouse(){
	return !enabled;
}

void Keyboard::GetKey(bool& currKBDown, std::string& currKBKey, touchPosition& touch){
	currKBDown = false;
	currKBKey = "";
	if(!enabled){
		return;
	}
	if (touch.py < KEY_YOFFSET){
		touchDown = false;
		repeatTimer = 0;
		repeatingKey = false;
		return;
	}
	
	keyX = std::min(std::max((touch.px - KEY_XOFFSET) / KEY_WIDTH,0),KEY_COLUMNS - 1);
	keyY = std::min(std::max((touch.py - KEY_YOFFSET) / KEY_HEIGHT,0),KEY_ROWS - 1);;
	
	if(touchDown){
		repeatTimer += tickSpeed;
		if(!repeatingKey){
			if(repeatTimer < REPEAT_WAIT1){
				return;
			}
			else {
				repeatTimer -= REPEAT_WAIT1;
				repeatingKey = true;
			}
		}
		else {
			if(repeatTimer < REPEAT_WAIT2){
				return;
			}
			else {
				repeatTimer -= REPEAT_WAIT2;
			}
		}
	}
	touchDown = true;
	
	
	//Logger_Write("%d %d\n", keyX,keyY);
	keyStr = layout[keyY][keyX][0];
	keyStrShift = layout[keyY][keyX][1];
	
	if(keyStr == "!S"){ //shift key
		shift = !shift;
	}
	else if(keyStr == "!C"){ //ctrl key
		ctrl = !ctrl;
	}
	else if(keyStr == "!A"){ //alt key
		alt = !alt;
	}
	else if(keyStr == "!E"){ //symbol key
		symbol = !symbol;
	}	
	else if(keyStr == ".."){ //empty key
		shift = false;
		ctrl = false;
		alt = false;
		symbol = false;
	}
	else{ //other keys
		if(symbol){
			currKBKey = charset::upper_to_emoji(keyStrShift);
			symbol = false;
		}
		else if(shift){
			currKBKey = keyStrShift;
			shift = false;
		}
		else{
			currKBKey = keyStr;
		}
		ctrl = false; //pico 8 cannot actually access modifier keys, might be worth adding as an optional feature?
		alt = false; 
		currKBDown = true;
	}
		
	
}


void Keyboard::DarkenRect(int x, int y, int w){
	int rectx = x*KEY_WIDTH+KEY_XOFFSET;
	int recty = y*KEY_HEIGHT+KEY_YOFFSET;
	int rectw = w*KEY_WIDTH;
	
	if(x == 0){
		rectx = 0;
		rectw += KEY_XOFFSET;
	}
	
	if(x + w == KEY_COLUMNS){
		rectw += KEY_XOFFSET;
	}
	
	C2D_DrawRectSolid(rectx, recty, 1, rectw, KEY_HEIGHT,darkenColor);
}

void Keyboard::Draw(){
	//Logger_Write("Drawing keyboard\n");
	if(!enabled){
		return;
		
	}
	C2D_DrawSprite(&kbSprite);
	
	if(touchDown){
		DarkenRect(keyX,keyY,1);
	}
	
	if(shift){
		DarkenRect(0,4,3);
	}
	if(ctrl){
		DarkenRect(8,4,2);
	}
	if(alt){
		DarkenRect(10,4,2);
	}
	if(symbol){
		DarkenRect(2,5,2);
	}
	
}

void Keyboard::Cleanup(){
	C2D_SpriteSheetFree(spritesheet);
}
#include "Keyboard.h"

#include <citro2d.h>

#include "../../source/logger.h"
#include "../../source/host.h"

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
		return;
	}
	
	currKBDown = true;
	currKBKey = "A";
}


void Keyboard::Draw(){
	//Logger_Write("Drawing keyboard\n");
	if(enabled){
		C2D_DrawSprite(&kbSprite);
	}

}

void Keyboard::Cleanup(){
	C2D_SpriteSheetFree(spritesheet);
}
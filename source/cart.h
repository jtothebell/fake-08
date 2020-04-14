#pragma once

#include <string>

#include "graphics.h"


struct PicoCart {
    std::string Filename;

    std::string FullCartString;

    std::string SpriteSheetString;

    std::string SpriteFlagsString;

    std::string MapString;
    
    std::string LuaScript;


    //SpriteSheet CartSpriteSheet;
    //uint8_t map_data[128 * 128];
};


void LoadCart(std::string filename);
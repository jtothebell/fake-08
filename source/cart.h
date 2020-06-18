#pragma once

#include <string>

#include "graphics.h"


class Cart {
    std::string fullCartText;

    void initCartRom();

    void setSpriteSheet(std::string spriteSheetString);
	void setSpriteFlags(std::string spriteFlagsString);
	void setMapData(std::string mapString);

    void setSfx(std::string sfxString);
    void setMusic(std::string musicString);

    bool loadCartFromPng(std::string filename);

    public:
    Cart (std::string filename);
    ~Cart();

    std::string Filename;

    std::string LuaString;

    std::string LoadError;

    std::string SpriteSheetString;
    uint8_t SpriteSheetData[128 * 64];

    std::string SpriteFlagsString;
    uint8_t SpriteFlagsData[256];

    std::string MapString;
    uint8_t MapData[128 * 32];

    std::string SfxString;
    struct sfx SfxData[64];

    std::string MusicString;
    struct song SongData[64];

    uint8_t CartLuaData[15616];

};

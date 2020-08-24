#include <string>

#include "doctest.h"
#include "../source/graphics.h"
#include "../source/fontdata.h"
#include "../source/PicoRam.h"

bool colorsEqual(Color* lhs, Color* rhs) {
	return lhs->Alpha == rhs->Alpha &&
		   lhs->Red == rhs->Red &&
		   lhs->Green == rhs->Green &&
		   lhs->Blue == rhs->Blue;
}

TEST_CASE("graphics class behaves as expected") {
    //general setup
    std::string fontdata = get_font_data();
    PicoRam picoRam;
    picoRam = {0};
    Graphics* graphics = new Graphics(fontdata, &picoRam);

    SUBCASE("Palette set up in constructor") {
        Color* palette = graphics->GetPaletteColors();
        Color testColor;
        testColor = {  0,   0,   0, 255};
        CHECK(colorsEqual(& palette[0], & testColor) == true);
        testColor = { 29,  43,  83, 255};
        CHECK(colorsEqual(& palette[1], & testColor) == true);
        testColor = {126,  37,  83, 255};
        CHECK(colorsEqual(& palette[2], & testColor) == true);
        testColor = {  0, 135,  81, 255};
        CHECK(colorsEqual(& palette[3], & testColor) == true);
        testColor = {171,  82,  54, 255};
        CHECK(colorsEqual(& palette[4], & testColor) == true);
        testColor = { 95,  87,  79, 255};
        CHECK(colorsEqual(& palette[5], & testColor) == true);
        testColor = {194, 195, 199, 255};
        CHECK(colorsEqual(& palette[6], & testColor) == true);
        testColor = {255, 241, 232, 255};
        CHECK(colorsEqual(& palette[7], & testColor) == true);
        testColor = {255,   0,  77, 255};
        CHECK(colorsEqual(& palette[8], & testColor) == true);
        testColor = {255, 163,   0, 255};
        CHECK(colorsEqual(& palette[9], & testColor) == true);
        testColor = {255, 240,  36, 255};
        CHECK(colorsEqual(& palette[10], & testColor) == true);
        testColor = {  0, 231,  86, 255};
        CHECK(colorsEqual(& palette[11], & testColor) == true);
        testColor = { 41, 173, 255, 255};
        CHECK(colorsEqual(& palette[12], & testColor) == true);
        testColor = {131, 118, 156, 255};
        CHECK(colorsEqual(& palette[13], & testColor) == true);
        testColor = {255, 119, 168, 255};
        CHECK(colorsEqual(& palette[14], & testColor) == true);
        testColor = {255, 204, 170, 255};
        CHECK(colorsEqual(& palette[15], & testColor) == true);
    }
    SUBCASE("Constructor sets default clip") {
        CHECK(picoRam._gfxState_clip_xb == 0);
        CHECK(picoRam._gfxState_clip_yb == 0);
        CHECK(picoRam._gfxState_clip_xe == 127);
        CHECK(picoRam._gfxState_clip_ye == 127);
    }
    SUBCASE("Constructor sets default color") {
        CHECK(picoRam._gfxState_color == 7);
    }
    SUBCASE("Constructor sets default draw color palette") {
        //colors mapped to themselves
        for (uint8_t c = 0; c < 16; c++) {
            CHECK(graphics->getPalMappedColor(c) == c);
        }
    }
    SUBCASE("Constructor sets default draw transparency palette") {
        //color 0 defaults to transparent
        CHECK(graphics->isColorTransparent(0) == true);

        //all other colors not transparent
        for (uint8_t c = 1; c < 16; c++) {
            CHECK(graphics->isColorTransparent(c) == false);
        }
    }
    SUBCASE("Constructor sets default screen palette") {
        
    }
    //check palette maps are set to default values

    //general teardown
    delete graphics;
}
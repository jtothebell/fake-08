#include <string>

#include "doctest.h"
#include "../source/graphics.h"
#include "../source/fontdata.h"
#include "../source/PicoRam.h"

TEST_CASE("graphics class behaves as expected") {
    //general setup
    std::string fontdata = get_font_data();
    PicoRam picoRam;
    picoRam = {0};
    Graphics* graphics = new Graphics(fontdata, &picoRam);

    SUBCASE("Palette set up in constructor") {
        Color* palette = graphics->GetPaletteColors();

        CHECK(palette[0].Alpha == 255);
        CHECK(palette[0].Red ==     0);
        CHECK(palette[0].Green ==   0);
        CHECK(palette[0].Blue ==    0);
    }

    //general teardown
    delete graphics;
}
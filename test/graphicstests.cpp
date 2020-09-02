#include <string>
#include <vector>

#include "doctest.h"
#include "../source/graphics.h"
#include "../source/fontdata.h"
#include "../source/PicoRam.h"

struct coloredPoint {
    uint8_t x;
    uint8_t y;
    uint8_t c;
};

bool colorsEqual(Color* lhs, Color* rhs) {
	return lhs->Alpha == rhs->Alpha &&
		   lhs->Red == rhs->Red &&
		   lhs->Green == rhs->Green &&
		   lhs->Blue == rhs->Blue;
}

void checkPoints(Graphics* graphics, std::vector<coloredPoint> expectedPoints) {
    bool isCorrect = true;
    for(size_t i = 0; i < expectedPoints.size(); i++){
        auto toCheck = expectedPoints[i];
        isCorrect &= graphics->pget(toCheck.x, toCheck.y) == toCheck.c;
    }

    CHECK(isCorrect);
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
            CHECK(graphics->getDrawPalMappedColor(c) == c);
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
        //colors mapped to themselves
        for (uint8_t c = 0; c < 16; c++) {
            CHECK(graphics->getScreenPalMappedColor(c) == c);
        }
    }
    SUBCASE("cls() clears the pico 8 framebuffer to color, defaults to black") {
        const uint8_t testColor = 4;
        graphics->cls(testColor);
        bool isAllColor = true;
        for(int x = 0; x < 128; x++){
            for(int y = 0; y < 128; y++){
                isAllColor &= graphics->pget(0, 0) == testColor;
            }
        }
        CHECK(isAllColor);
    }
    SUBCASE("cls() defaults to black") {
        bool isAllColor = true;
        graphics->cls();
        for(int x = 0; x < 128; x++){
            for(int y = 0; y < 128; y++){
                isAllColor &= graphics->pget(0, 0) == 0;
            }
        }
        CHECK(isAllColor);
    }
    SUBCASE("pset() sets color at coord") {
        const uint8_t testColor = 15;
        graphics->pset(72, 31, testColor);

        CHECK(graphics->pget(72, 31) == testColor);
    }
    SUBCASE("pset() with no color uses pen color") {
        graphics->pset(121, 6);

        CHECK(graphics->pget(121, 6) == picoRam._gfxState_color);
    }
    SUBCASE("color({color}) sets color in ram") {
        graphics->color(12);

        CHECK(picoRam._gfxState_color == 12);
    }
    SUBCASE("line() clears line state") {
        picoRam._gfxState_line_x = 10;
	    picoRam._gfxState_line_y = 30;
	    picoRam._gfxState_line_valid = true;

        graphics->line();

        CHECK(
            (picoRam._gfxState_line_x == 0 && 
            picoRam._gfxState_line_y == 0 &&
            picoRam._gfxState_line_valid == false) == true);
    }
    SUBCASE("line({arg}) sets color and clears line") {
        picoRam._gfxState_line_x = 10;
	    picoRam._gfxState_line_y = 30;
	    picoRam._gfxState_line_valid = true;
        picoRam._gfxState_color = 2;

        graphics->line(14);

        CHECK(
            (picoRam._gfxState_line_x == 0 && 
            picoRam._gfxState_line_y == 0 &&
            picoRam._gfxState_line_valid == false &&
            picoRam._gfxState_color == 14) == true);
    }
    SUBCASE("line({x}, {y}) without valid line state does nothing") {
        graphics->cls();
        graphics->line();
        graphics->line(10, 11);

        bool isAllColor = true;
        for(int x = 0; x < 128; x++){
            for(int y = 0; y < 128; y++){
                isAllColor &= graphics->pget(0, 0) == 0;
            }
        }

        CHECK(
            (picoRam._gfxState_line_x == 0 && 
            picoRam._gfxState_line_y == 0 &&
            picoRam._gfxState_line_valid == false &&
            isAllColor) == true);
    }
    SUBCASE("line({x}, {y}) with valid line state updates state") {
        graphics->cls();
        picoRam._gfxState_line_x = 10;
        picoRam._gfxState_line_y = 10;
        picoRam._gfxState_line_valid = true;
        graphics->line(13, 13);

        CHECK(
            (picoRam._gfxState_line_x == 13 && 
            picoRam._gfxState_line_y == 13 &&
            picoRam._gfxState_line_valid == true) == true);
    }
    SUBCASE("line({x}, {y}) draws (45 degree down-right)") {
        graphics->cls();
        picoRam._gfxState_color = 2;
        picoRam._gfxState_line_x = 10;
        picoRam._gfxState_line_y = 10;
        picoRam._gfxState_line_valid = true;
        graphics->line(12, 12);

        std::vector<coloredPoint> expectedPoints = {
            {10, 10, 2},
            {11, 11, 2},
            {12, 12, 2}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x}, {y}, {c}) draws (vertical down)") {
        graphics->cls();
        picoRam._gfxState_line_x = 20;
        picoRam._gfxState_line_y = 20;
        picoRam._gfxState_line_valid = true;
        graphics->line(20, 24, 13);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 13},
            {20, 21, 13},
            {20, 22, 13},
            {20, 23, 13},
            {20, 24, 13}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}) draws (45 degree left)") {
        graphics->cls();
        picoRam._gfxState_color = 10;
        graphics->line(20, 20, 18, 22);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 10},
            {19, 21, 10},
            {18, 22, 10}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}, {c}) draws (horizontal left)") {
        graphics->cls();
        graphics->line(20, 20, 18, 20, 4);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 4},
            {19, 20, 4},
            {18, 20, 4}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}, {c}) draws (45 degree up left)") {
        graphics->cls();
        graphics->line(20, 20, 18, 18, 5);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 5},
            {19, 19, 5},
            {18, 18, 5}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}, {c}) draws (vertical up)") {
        graphics->cls();
        graphics->line(20, 20, 20, 18, 5);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 5},
            {20, 19, 5},
            {20, 18, 5}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}, {c}) draws (45 degree up right)") {
        graphics->cls();
        graphics->line(20, 20, 22, 18, 5);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 5},
            {21, 19, 5},
            {22, 18, 5}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}, {c}) draws (horizontal right)") {
        graphics->cls();
        graphics->line(20, 20, 22, 20, 5);

        std::vector<coloredPoint> expectedPoints = {
            {20, 20, 5},
            {21, 20, 5},
            {22, 20, 5}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circ({ox}, {oy}) uses pen color and radius of 4") {
        graphics->cls();
        graphics->circ(40, 40);

        //quarter circle from 12 oclock to 3 oclock
        std::vector<coloredPoint> expectedPoints = {
            {39, 36, 7},
            {40, 36, 7},
            {41, 36, 7},
            {42, 37, 7},
            {43, 37, 7},
            {43, 38, 7},
            {44, 39, 7},
            {44, 40, 7}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circ({ox}, {oy}, {r}) uses pen color") {
        graphics->cls();
        graphics->color(14);
        graphics->circ(40, 40, 1);

        //radius of 1 draws 4 points around center
        std::vector<coloredPoint> expectedPoints = {
            {40, 39, 14},
            {41, 40, 14},
            {40, 41, 14},
            {39, 40, 14},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circ({ox}, {oy}, {r}, {c}) draws correctly") {
        graphics->cls();
        graphics->circ(40, 40, 2, 13);

        //radius of 2 draws this circle
        std::vector<coloredPoint> expectedPoints = {
            {38, 39, 13},
            {38, 40, 13},
            {38, 41, 13},
            {39, 38, 13},
            {39, 42, 13},
            {40, 38, 13},
            {40, 42, 13},
            {41, 38, 13},
            {41, 42, 13},
            {42, 39, 13},
            {42, 40, 13},
            {42, 41, 13},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circ({ox}, {oy}, {r}, {c}) radius of 0 draws 1 point") {
        graphics->cls();
        graphics->circ(40, 40, 0, 13);

        //radius of 2 draws this circle
        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 13},
        };

        checkPoints(graphics, expectedPoints);
    }
    //circfill (same as circ tests, but also test center point)
    SUBCASE("circfill({ox}, {oy}) uses pen color and radius of 4") {
        graphics->cls();
        graphics->circfill(40, 40);

        //quarter circle from 12 oclock to 3 oclock
        std::vector<coloredPoint> expectedPoints = {
            {39, 36, 7},
            {40, 36, 7},
            {41, 36, 7},
            {42, 37, 7},
            {43, 37, 7},
            {43, 38, 7},
            {44, 39, 7},
            {44, 40, 7},
            {40, 40, 7},//center point
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circfill({ox}, {oy}, {r}) uses pen color") {
        graphics->cls();
        graphics->color(14);
        graphics->circfill(40, 40, 1);

        //radius of 1 draws 4 points around center, plus the center
        std::vector<coloredPoint> expectedPoints = {
            {40, 39, 14},
            {41, 40, 14},
            {40, 41, 14},
            {39, 40, 14},
            {40, 40, 14},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circfill({ox}, {oy}, {r}, {c}) draws correctly") {
        graphics->cls();
        graphics->circfill(40, 40, 2, 13);

        //radius of 2 draws this circle
        std::vector<coloredPoint> expectedPoints = {
            {38, 39, 13},
            {38, 40, 13},
            {38, 41, 13},
            {39, 38, 13},
            {39, 42, 13},
            {40, 38, 13},
            {40, 42, 13},
            {41, 38, 13},
            {41, 42, 13},
            {42, 39, 13},
            {42, 40, 13},
            {42, 41, 13},
            {40, 40, 13},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("circfill({ox}, {oy}, {r}, {c}) radius of 0 draws 1 point") {
        graphics->cls();
        graphics->circfill(40, 40, 0, 13);

        //radius of 0 draws a point
        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 13},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("rect({x1}, {y1}, {x2}, {y2}) uses pen color") {
        graphics->cls();
        graphics->color(15);
        graphics->rect(40, 40, 43, 42);

        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 15},
            {40, 41, 15},
            {40, 42, 15},
            {41, 40, 15},
            {41, 42, 15},
            {42, 40, 15},
            {42, 42, 15},
            {43, 40, 15},
            {43, 41, 15},
            {43, 42, 15}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("rect({x1}, {y1}, {x2}, {y2}, {c}) swapped coords work, color used") {
        graphics->cls();
        graphics->rect(42, 43, 40, 40, 1);

        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 1},
            {40, 41, 1},
            {40, 42, 1},
            {40, 43, 1},
            {41, 40, 1},
            {41, 43, 1},
            {42, 40, 1},
            {42, 41, 1},
            {42, 42, 1},
            {42, 43, 1}
        };

        checkPoints(graphics, expectedPoints);
    }

    

    //general teardown
    delete graphics;
}
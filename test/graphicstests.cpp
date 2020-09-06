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

void debugScreen(Graphics* graphics) {
    for (int x=0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
 	        uint8_t c = graphics->pget(x,y);
            if (c != 0) {
                printf("%d,%d,%d\n", x, y, c);
            }
        }
    }
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
    SUBCASE("rectfill({x1}, {y1}, {x2}, {y2}) uses pen color, fills rect") {
        graphics->cls();
        graphics->color(10);
        graphics->rectfill(40, 40, 43, 43);

        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 10},
            {40, 41, 10},
            {40, 42, 10},
            {40, 43, 10},
            {41, 40, 10},
            {41, 41, 10},
            {41, 42, 10},
            {41, 43, 10},
            {42, 40, 10},
            {42, 41, 10},
            {42, 42, 10},
            {42, 43, 10},
            {43, 40, 10},
            {43, 41, 10},
            {43, 42, 10},
            {43, 43, 10}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("rectfill({x1}, {y1}, {x2}, {y2}, {c}) swapped coords work, color used") {
        graphics->cls();
        graphics->rectfill(42, 43, 40, 40, 2);

        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 2},
            {40, 41, 2},
            {40, 42, 2},
            {40, 43, 2},
            {41, 40, 2},
            {41, 41, 2},
            {41, 42, 2},
            {41, 43, 2},
            {42, 40, 2},
            {42, 41, 2},
            {42, 42, 2},
            {42, 43, 2}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("print({str}) uses current color, ignoring transparency") {
        graphics->cls();
        graphics->color(2);
        graphics->palt(2, true);

        graphics->print("t");

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 2},
            {1, 0, 2},
            {2, 0, 2},
            {1, 1, 2},
            {1, 2, 2},
            {1, 3, 2},
            {1, 4, 2}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("print({str}) uses current text location") {
        graphics->cls();
        graphics->color(3);
        picoRam._gfxState_text_x = 15;
        picoRam._gfxState_text_y = 98;

        graphics->print("t");

        std::vector<coloredPoint> expectedPoints = {
            {15, 98, 3},
            {16, 98, 3},
            {17, 98, 3},
            {16, 99, 3},
            {16, 100, 3},
            {16, 101, 3},
            {16, 102, 3}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("print({str}) increments text location y by 6") {
        graphics->cls();
        picoRam._gfxState_text_x = 15;
        picoRam._gfxState_text_y = 110;

        graphics->print("doesnt matter");

        CHECK(picoRam._gfxState_text_y == 116);
    }
    SUBCASE("print({str}, {x}, {y}) updates text location ") {
        graphics->cls();
        picoRam._gfxState_text_x = 3;
        picoRam._gfxState_text_y = 4;

        graphics->print("doesnt matter", 42, 99);

        CHECK(picoRam._gfxState_text_x == 42);
        CHECK(picoRam._gfxState_text_y == 99);
    }
    SUBCASE("print({str}, {x}, {y}, {c}) updates text location and color") {
        graphics->cls();
        picoRam._gfxState_text_x = 3;
        picoRam._gfxState_text_y = 4;
        picoRam._gfxState_color = 10;

        graphics->print("doesnt matter", 16, 18, 14);
        
        CHECK(picoRam._gfxState_text_x == 16);
        CHECK(picoRam._gfxState_text_y == 18);
        CHECK(picoRam._gfxState_color == 14);
    }
    SUBCASE("spr(...) draws to screen at location") {
        graphics->cls();
        for(uint8_t i = 0; i < 16; i++) {
            graphics->sset(i % 8, i / 8, i%2 == 0 ? i : 0 );
        }

        graphics->spr(0, 101, 33, 1.0, 1.0, false, false);
        
        std::vector<coloredPoint> expectedPoints = {
            {103, 33, 2},
            {105, 33, 4},
            {107, 33, 6},
            {101, 34, 8},
            {103, 34, 10},
            {105, 34, 12},
            {107, 34, 14},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("spr(...) draws more than 1 horizontal sprite") {
        graphics->cls();
        for(uint8_t i = 0; i < 16; i++) {
            graphics->sset(i, 0, i);
        }

        graphics->spr(0, 35, 100, 1.5, 1.0, false, false);
        
        std::vector<coloredPoint> expectedPoints = {
            {35, 100, 0},
            {36, 100, 1},
            {37, 100, 2},
            {38, 100, 3},
            {39, 100, 4},
            {40, 100, 5},
            {41, 100, 6},
            {42, 100, 7},
            {43, 100, 8},
            {44, 100, 9},
            {45, 100, 10},
            {46, 100, 11},
            {47, 100, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("spr(...) draws more than 1 vertical sprite") {
        graphics->cls();
        for(uint8_t i = 0; i < 16; i++) {
            graphics->sset(0, i, i);
        }

        graphics->spr(0, 35, 100, 1.0, 1.25, false, false);
        
        std::vector<coloredPoint> expectedPoints = {
            {35, 100, 0},
            {35, 101, 1},
            {35, 102, 2},
            {35, 103, 3},
            {35, 104, 4},
            {35, 105, 5},
            {35, 106, 6},
            {35, 107, 7},
            {35, 108, 8},
            {35, 109, 9},
            {35, 100, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("spr(...) draws flipped Horizontal") {
        graphics->cls();
        for(uint8_t i = 0; i < 16; i++) {
            graphics->sset(i, 0, i);
        }

        graphics->spr(0, 35, 100, 1.0, 1.0, true, false);
        
        std::vector<coloredPoint> expectedPoints = {
            {34, 100, 0},
            {35, 100, 7},
            {36, 100, 6},
            {37, 100, 5},
            {38, 100, 4},
            {39, 100, 3},
            {40, 100, 2},
            {41, 100, 1},
            {42, 100, 0},
            {43, 100, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("spr(...) draws flipped vertical") {
        graphics->cls();
        for(uint8_t i = 0; i < 16; i++) {
            graphics->sset(0, i, i);
        }

        graphics->spr(0, 35, 100, 1.0, 1.0, false, true);
        
        std::vector<coloredPoint> expectedPoints = {
            {35, 99, 0},
            {35, 100, 7},
            {35, 101, 6},
            {35, 102, 5},
            {35, 103, 4},
            {35, 104, 3},
            {35, 105, 2},
            {35, 106, 1},
            {35, 107, 0},
            {35, 108, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("spr(...) draws to screen at odd numbered location") {
        graphics->cls();
        
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }

        graphics->spr(0, 51, 11, 1, 1, false, false);

        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {50, 10, 0},
            {51, 11, 0},
            {52, 12, 1},
            {53, 13, 2},
            {54, 14, 3},
            {55, 15, 4},
            {56, 16, 5},
            {57, 17, 6},
            {58, 18, 7},
            {59, 19, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("sspr(...) draws to screen at odd numbered location") {
        graphics->cls();
        
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }

        graphics->sspr(0, 0, 8, 8, 51, 11, 8, 8, false, false);

        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {50, 10, 0},
            {51, 11, 0},
            {52, 12, 1},
            {53, 13, 2},
            {54, 14, 3},
            {55, 15, 4},
            {56, 16, 5},
            {57, 17, 6},
            {58, 18, 7},
            {59, 19, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("sspr(...) draws from odd numbered sprite sheet location") {
        graphics->cls();
        
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }

        graphics->sspr(1, 1, 4, 4, 51, 11, 4, 4, false, false);

        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {51, 10, 0},
            {51, 11, 1},
            {51, 12, 1},
            {51, 13, 1},
            {51, 14, 1},
            {52, 11, 2},
            {52, 12, 2},
            {52, 13, 2},
            {52, 14, 2},
            {53, 11, 3},
            {53, 12, 3},
            {53, 13, 3},
            {53, 14, 3},
            {54, 11, 4},
            {54, 12, 4},
            {54, 13, 4},
            {54, 14, 4},
            {55, 11, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("sspr(...) draws non-square to screen at location") {
        graphics->cls();
        
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }

        graphics->sspr(3, 2, 3, 4, 100, 50, 3, 4, false, false);

        std::vector<coloredPoint> expectedPoints = {
            {100, 50, 3},
            {100, 51, 3},
            {100, 52, 3},
            {100, 53, 3},
            {101, 50, 4},
            {101, 51, 4},
            {101, 52, 4},
            {101, 53, 4},
            {102, 50, 5},
            {102, 51, 5},
            {102, 52, 5},
            {102, 53, 5},
            
        };

        checkPoints(graphics, expectedPoints);
    }
    /*//TODO: come back and make SSPR stretch exactly how pico 8 does
    SUBCASE("sspr(...) draws unevenly stretched sprite") {
        graphics->cls();
        
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }

        graphics->sspr(3,1,3,1,100,50,5,1,false,false);

        debugScreen(graphics);

        std::vector<coloredPoint> expectedPoints = {
            {100, 50, 3},
            {101, 50, 3},
            {102, 50, 4},
            {103, 50, 5},
            {104, 50, 5},
        };

        checkPoints(graphics, expectedPoints);
    }
    */
   SUBCASE("fget for sprite with none set")
   {
       auto result = graphics->fget(0);

       CHECK_EQ(result, 0);
   }
   SUBCASE("fget for sprite with flags set")
   {
       uint8_t flags = 47;
       picoRam.spriteFlags[4] = flags;
       auto result = graphics->fget(4);

       CHECK_EQ(result, flags);
   }
   SUBCASE("fget for sprite and flag number with flags set")
   {
       //118 in binary: 0111 0110
       uint8_t flags = 118;
       picoRam.spriteFlags[12] = flags;
       auto result0 = graphics->fget(12, 0);
       auto result1 = graphics->fget(12, 1);
       auto result2 = graphics->fget(12, 2);
       auto result3 = graphics->fget(12, 3);
       auto result4 = graphics->fget(12, 4);
       auto result5 = graphics->fget(12, 5);
       auto result6 = graphics->fget(12, 6);
       auto result7 = graphics->fget(12, 7);

       bool offBitsCorrect = result0 == false && result3 == false && result7 == false;
       bool onBitsCorrect = result1 == true && result2 == true && result4 == true
                         && result5 == true && result6 == true;

       CHECK(offBitsCorrect);
       CHECK(onBitsCorrect);
   }
   SUBCASE("fset sets value in ram")
   {
       uint8_t flags = 65;
       graphics->fset(31, flags);
       auto result = picoRam.spriteFlags[31];

       CHECK_EQ(result, flags);
   }
   SUBCASE("fset for single flag sets value in ram")
   {
       //fifth bit and second bit 0010 0100 == 36 in decimal
       graphics->fset(17, 5, true);
       graphics->fset(17, 2, true);

       auto result = picoRam.spriteFlags[17];

       CHECK_EQ(result, 36);
   }
   SUBCASE("sget defaults to 0 when nothing on sprite sheet")
   {
       auto result = graphics->sget(14, 41);

       CHECK_EQ(result, 0);
   }
   SUBCASE("sget returns correct value for even x pixel")
   {
       int x = 40;
       int y = 10;
       int combinedIdx = y * 64 + (x / 2);
       picoRam.spriteSheetData[combinedIdx] = 137; //10001001
       auto result = graphics->sget(x, y);

       CHECK_EQ(result, 9);
   }
   SUBCASE("sget returns correct value for odd x pixel")
   {
       int x = 41;
       int y = 11;
       int combinedIdx = y * 64 + (x / 2);
       picoRam.spriteSheetData[combinedIdx] = 200; //11001000
       auto result = graphics->sget(x, y);

       CHECK_EQ(result, 12);
   }
   SUBCASE("sset sets value in ram for even x pixel")
   {
       uint8_t x = 60;
       uint8_t y = 23;
       graphics->sset(x, y, 14); //14 = 1110

       int combinedIdx = y * 64 + (x / 2);
       auto result = picoRam.spriteSheetData[combinedIdx];

       CHECK_EQ(result, 14);
   }
   SUBCASE("sset sets value in ram for odd x pixel")
   {
       uint8_t x = 61;
       uint8_t y = 23;
       graphics->sset(x, y, 12); //12 = 1100

       int combinedIdx = y * 64 + (x / 2);
       auto result = picoRam.spriteSheetData[combinedIdx];

       CHECK_EQ(result, 192);
   }
   SUBCASE("camera({x}, {y}) sets values in memory")
   {
       uint8_t x = 33;
       uint8_t y = 120;
       graphics->camera(x, y);

       CHECK_EQ(picoRam._gfxState_camera_x, x);
       CHECK_EQ(picoRam._gfxState_camera_y, y);
   }
   SUBCASE("camera() sets values in memory to 0")
   {
       picoRam._gfxState_camera_x = 33;
       picoRam._gfxState_camera_y = 120;
       graphics->camera();

       CHECK_EQ(picoRam._gfxState_camera_x, 0);
       CHECK_EQ(picoRam._gfxState_camera_y, 0);
   }
   //TODO: check drawing calls to make sure these values honored
   SUBCASE("camera values applies to pget")
   {
       uint8_t col = 13;
       picoRam._gfxState_camera_x = -44;
       picoRam._gfxState_camera_y = -3;
       graphics->pset(14, 32, 13);
       
       auto result = graphics->pget(14, 32);

       CHECK_EQ(result, col);
   }
   SUBCASE("camera values applies to pset")
   {
       uint8_t col = 13;
       picoRam._gfxState_camera_x = -44;
       picoRam._gfxState_camera_y = -3;
       graphics->pset(14, 32, 13);
       graphics->camera();
       
       auto result = graphics->pget(58, 35);

       CHECK_EQ(result, col);
   }

   


    

    //general teardown
    delete graphics;
}
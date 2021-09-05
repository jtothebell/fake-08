#include <string>
#include <vector>
#include <tuple>

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
    picoRam.Reset();
    Graphics* graphics = new Graphics(fontdata, &picoRam);
    graphics->cls();

    SUBCASE("Palette set up in constructor") {
        Color* palette = graphics->GetPaletteColors();
        Color testColor;
        testColor = {  2,   4,   8, 255};
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
        testColor = {255, 236,  39, 255};
        CHECK(colorsEqual(& palette[10], & testColor) == true);
        testColor = {  0, 228,  54, 255};
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
        CHECK(picoRam.drawState.clip_xb == 0);
        CHECK(picoRam.drawState.clip_yb == 0);
        CHECK(picoRam.drawState.clip_xe == 128);
        CHECK(picoRam.drawState.clip_ye == 128);
    }
    SUBCASE("Constructor sets default color") {
        CHECK(picoRam.drawState.color == 6);
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
                isAllColor &= graphics->pget(x, y) == testColor;
            }
        }
        CHECK(isAllColor);
    }
    SUBCASE("cls() defaults to black") {
        bool isAllColor = true;
        graphics->cls();
        for(int x = 0; x < 128; x++){
            for(int y = 0; y < 128; y++){
                isAllColor &= graphics->pget(x, y) == 0;
            }
        }
        CHECK(isAllColor);
    }
    SUBCASE("cls() resets cursor") {
        picoRam.drawState.text_x = 42;
        picoRam.drawState.text_y = 99;
        graphics->cls();
        graphics->cursor();

        CHECK_EQ(picoRam.drawState.text_x, 0);
        CHECK_EQ(picoRam.drawState.text_y, 0);
    }
    SUBCASE("cls() resets clip") {
        picoRam.drawState.clip_xb = 10;
        picoRam.drawState.clip_yb = 12;
        picoRam.drawState.clip_xe = 100;
        picoRam.drawState.clip_ye = 102;
        graphics->cls();

        CHECK_EQ(picoRam.drawState.clip_xb, 0);
        CHECK_EQ(picoRam.drawState.clip_yb, 0);
        CHECK_EQ(picoRam.drawState.clip_xe, 128);
        CHECK_EQ(picoRam.drawState.clip_ye, 128);
    }
    SUBCASE("pset() sets color at coord") {
        const uint8_t testColor = 15;
        graphics->pset(72, 31, testColor);

        CHECK(graphics->pget(72, 31) == testColor);
    }
    SUBCASE("pset() with no color uses pen color") {
        graphics->pset(121, 6);

        CHECK(graphics->pget(121, 6) == picoRam.drawState.color);
    }
    SUBCASE("color({color}) sets color in ram") {
        graphics->color(12);

        CHECK(picoRam.drawState.color == 12);
    }
    SUBCASE("color() resets color to 6") {
        picoRam.drawState.color = 12;

        graphics->color();

        CHECK(picoRam.drawState.color == 6);
    }
    SUBCASE("color() returns previous") {
        auto prev = picoRam.drawState.color = 13;

        graphics->color(12);

        CHECK(prev == 13);
    }
    SUBCASE("line() clears line state") {
        picoRam.drawState.line_x = 10;
	    picoRam.drawState.line_y = 30;
	    picoRam.drawState.lineInvalid = false;

        graphics->line();

        CHECK(
            (picoRam.drawState.line_x == 0 && 
            picoRam.drawState.line_y == 0 &&
            picoRam.drawState.lineInvalid == true) == true);
    }
    SUBCASE("line({arg}) sets color and clears line") {
        picoRam.drawState.line_x = 10;
	    picoRam.drawState.line_y = 30;
	    picoRam.drawState.lineInvalid = false;
        picoRam.drawState.color = 2;

        graphics->line(14);

        CHECK(
            (picoRam.drawState.line_x == 0 && 
            picoRam.drawState.line_y == 0 &&
            picoRam.drawState.lineInvalid == true &&
            picoRam.drawState.color == 14) == true);
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
            (picoRam.drawState.line_x == 0 && 
            picoRam.drawState.line_y == 0 &&
            picoRam.drawState.lineInvalid == true &&
            isAllColor) == true);
    }
    SUBCASE("line({x}, {y}) with valid line state updates state") {
        graphics->cls();
        picoRam.drawState.line_x = 10;
        picoRam.drawState.line_y = 10;
        picoRam.drawState.lineInvalid = false;
        graphics->line(13, 13);

        CHECK(
            (picoRam.drawState.line_x == 13 && 
            picoRam.drawState.line_y == 13 &&
            picoRam.drawState.lineInvalid == false) == true);
    }
    SUBCASE("line({x}, {y}) draws (45 degree down-right)") {
        graphics->cls();
        picoRam.drawState.color = 2;
        picoRam.drawState.line_x = 10;
        picoRam.drawState.line_y = 10;
        picoRam.drawState.lineInvalid = false;
        graphics->line(12, 12);

        std::vector<coloredPoint> expectedPoints = {
            {10, 10, 2},
            {11, 11, 2},
            {12, 12, 2}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x}, {y}, {c}) draws (vertical down, even x value)") {
        graphics->cls();
        picoRam.drawState.line_x = 20;
        picoRam.drawState.line_y = 20;
        picoRam.drawState.lineInvalid = false;
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
    SUBCASE("line({x}, {y}, {c}) draws (vertical up, odd x value)") {
        graphics->cls();
        picoRam.drawState.line_x = 21;
        picoRam.drawState.line_y = 24;
        picoRam.drawState.lineInvalid = false;
        graphics->line(21, 20, 3);

        std::vector<coloredPoint> expectedPoints = {
            {21, 20, 3},
            {21, 21, 3},
            {21, 22, 3},
            {21, 23, 3},
            {21, 24, 3}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("line({x1}, {y1}, {x2}, {y2}) draws (45 degree left)") {
        graphics->cls();
        picoRam.drawState.color = 10;
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
    SUBCASE("overdrawn line() still draws (horizontal)") {
        graphics->cls();
        graphics->line(-10, 1, 138, 1, 5);

        std::vector<coloredPoint> expectedPoints = {
            {0, 1, 5},
            {1, 1, 5},
            {2, 1, 5}//etc
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("overdrawn line() still draws (vertical)") {
        graphics->cls();
        graphics->line(1, -10, 1, 138, 5);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 5},
            {1, 1, 5},
            {1, 2, 5}//etc
        };

        checkPoints(graphics, expectedPoints);
    }

    SUBCASE("circ({ox}, {oy}) uses pen color and radius of 4") {
        graphics->cls();
        graphics->circ(40, 40);

        //quarter circle from 12 oclock to 3 oclock
        std::vector<coloredPoint> expectedPoints = {
            {39, 36, 6},
            {40, 36, 6},
            {41, 36, 6},
            {42, 37, 6},
            {43, 37, 6},
            {43, 38, 6},
            {44, 39, 6},
            {44, 40, 6}
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
            {39, 36, 6},
            {40, 36, 6},
            {41, 36, 6},
            {42, 37, 6},
            {43, 37, 6},
            {43, 38, 6},
            {44, 39, 6},
            {44, 40, 6},
            {40, 40, 6},//center point
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
    SUBCASE("oval({x0}, {x1}, {y0}, {y1}, {c}) draws an ellipse") {
        graphics->cls();
        graphics->oval(40, 40, 45, 42, 13);

        std::vector<coloredPoint> expectedPoints = {
            {40,41,13},
            {41,40,13},
            {41,42,13},
            {42,40,13},
            {42,42,13},
            {43,40,13},
            {43,42,13},
            {44,41,13}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("oval({x0}, {x1}, {y0}, {y1}, {c}) with zeroes doesn't infinite loop") {
        graphics->cls();
        graphics->oval(40, 40, 40, 40, 13);

        std::vector<coloredPoint> expectedPoints = {};

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("ovalfill({x0}, {x1}, {y0}, {y1}, {c}) fills an ellipse") {
        graphics->cls();
        graphics->ovalfill(40, 40, 45, 42, 13);

        std::vector<coloredPoint> expectedPoints = {
            {40,41,13},
            {41,40,13},
            {41,41,13},
            {41,42,13},
            {42,40,13},
            {42,41,13},
            {42,42,13},
            {43,40,13},
            {43,41,13},
            {43,42,13},
            {44,41,13}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("ovalfill({x0}, {x1}, {y0}, {y1}, {c}) with zeroes doesn't infinite loop") {
        graphics->cls();
        graphics->ovalfill(40, 40, 40, 40, 13);

        std::vector<coloredPoint> expectedPoints = {};

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
    SUBCASE("rect({x1}, {y1}, {x2}, {y2}) offscreen x draws nothing") {
        graphics->cls();
        graphics->color(15);
        graphics->rect(-50, 40, -40, 42);

        std::vector<coloredPoint> expectedPoints = {
            {0, 40, 0},
            {0, 41, 0},
            {0, 42, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("rect({x1}, {y1}, {x2}, {y2}) offscreen y draws nothing") {
        graphics->cls();
        graphics->color(15);
        graphics->rect(40, -50, 43, -40);

        std::vector<coloredPoint> expectedPoints = {
            {40, 0, 0},
            {41, 0, 0},
            {42, 0, 0},
            {43, 0, 0},
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
        picoRam.drawState.text_x = 15;
        picoRam.drawState.text_y = 98;

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
        picoRam.drawState.text_x = 15;
        picoRam.drawState.text_y = 110;

        graphics->print("doesnt matter");

        CHECK(picoRam.drawState.text_y == 116);
    }
    SUBCASE("print({str}, {x}, {y}) updates text location ") {
        graphics->cls();
        picoRam.drawState.text_x = 3;
        picoRam.drawState.text_y = 4;

        graphics->print("doesnt matter", 42, 99);

        CHECK(picoRam.drawState.text_x == 42);
        CHECK(picoRam.drawState.text_y == 105);
    }
    SUBCASE("print({str}, {x}, {y}, {c}) updates text location and color") {
        graphics->cls();
        picoRam.drawState.text_x = 3;
        picoRam.drawState.text_y = 4;
        picoRam.drawState.color = 10;

        graphics->print("doesnt matter", 16, 18, 14);
        
        CHECK(picoRam.drawState.text_x == 16);
        CHECK(picoRam.drawState.text_y == 24);
        CHECK(picoRam.drawState.color == 14);
    }
    SUBCASE("print({str}) uses pal mapped color") {
        graphics->cls();
        graphics->color(2);
        graphics->pal(2, 12, 0);

        graphics->print("t");

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 12},
            {1, 0, 12},
            {2, 0, 12},
            {1, 1, 12},
            {1, 2, 12},
            {1, 3, 12},
            {1, 4, 12}
        };

        checkPoints(graphics, expectedPoints);
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
    SUBCASE("sspr(...) negative screen dimensions flip image") {
        graphics->cls();
        
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }

        graphics->sspr(3, 2, 3, 4, 100, 50, 3, -4, false, false);

        std::vector<coloredPoint> expectedPoints = {
            {100, 46, 3},
            {100, 47, 3},
            {100, 48, 3},
            {100, 49, 3},
            {101, 46, 4},
            {101, 47, 4},
            {101, 48, 4},
            {101, 49, 4},
            {102, 46, 5},
            {102, 47, 5},
            {102, 48, 5},
            {102, 49, 5},
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

       CHECK_EQ(picoRam.drawState.camera_x, x);
       CHECK_EQ(picoRam.drawState.camera_y, y);
   }
   SUBCASE("camera() sets values in memory to 0")
   {
       picoRam.drawState.camera_x = 33;
       picoRam.drawState.camera_y = 120;
       graphics->camera();

       CHECK_EQ(picoRam.drawState.camera_x, 0);
       CHECK_EQ(picoRam.drawState.camera_y, 0);
   }
   SUBCASE("camera({x}, {y}) returns previous values")
   {
       uint8_t x = 33;
       uint8_t y = 120;
       graphics->camera(x, y);
       auto prev = graphics->camera(120, 31);

       CHECK_EQ(std::get<0>(prev), x);
       CHECK_EQ(std::get<1>(prev), y);
   }
   SUBCASE("camera values applies to pget")
   {
       uint8_t col = 13;
       picoRam.drawState.camera_x = -44;
       picoRam.drawState.camera_y = -3;
       graphics->pset(14, 32, 13);
       
       auto result = graphics->pget(14, 32);

       CHECK_EQ(result, col);
   }
   SUBCASE("camera values applies to pset")
   {
       uint8_t col = 13;
       picoRam.drawState.camera_x = -44;
       picoRam.drawState.camera_y = -3;
       graphics->pset(14, 32, 13);
       graphics->camera();
       
       auto result = graphics->pget(58, 35);

       CHECK_EQ(result, col);
   }
   SUBCASE("camera values apply to line")
   {
        graphics->cls();
        graphics->camera(-10, -10);
        graphics->line(20, 20, 18, 22, 10);
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {30, 30, 10},
            {29, 31, 10},
            {28, 32, 10}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to circ")
    {
        graphics->cls();
        graphics->camera(-20, -20);
        graphics->circ(40, 40, 1, 14);
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {60, 59, 14},
            {61, 60, 14},
            {60, 61, 14},
            {59, 60, 14},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to circfill")
    {
        graphics->cls();
        graphics->camera(-30, -30);
        graphics->circfill(40, 40, 1, 3);
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {70, 70, 3},
            {70, 69, 3},
            {71, 70, 3},
            {70, 71, 3},
            {69, 70, 3},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to rect")
    {
        graphics->cls();
        graphics->camera(-50, -50);
        graphics->rect(-10, -10, -8, -8, 4);
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {40, 40, 4},
            {41, 40, 4},
            {42, 40, 4},
            {40, 41, 4},
            {40, 42, 4},
            {40, 42, 4},
            {41, 42, 4},
            {42, 42, 4},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to rectfill")
    {
        graphics->cls();
        graphics->camera(-100, -100);
        graphics->rectfill(0, 0, 1, 1, 3);
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {100, 100, 3},
            {100, 101, 3},
            {101, 100, 3},
            {101, 101, 3},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to print")
    {
        graphics->cls();
        graphics->camera(-100, -100);
        graphics->print("t");
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {100, 100, 6},
            {101, 100, 6},
            {102, 100, 6},
            {101, 101, 6},
            {101, 102, 6},
            {101, 103, 6},
            {101, 104, 6}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to spr")
    {
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }
        
        graphics->cls();
        graphics->camera(-100, -100);
        graphics->spr(0, 10, 10, 1, 1, false, false);
        graphics->camera();

        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {109, 109, 0},
            {110, 110, 0},
            {111, 111, 1},
            {112, 112, 2},
            {113, 113, 3},
            {114, 114, 4},
            {115, 115, 5},
            {116, 116, 6},
            {117, 117, 7},
            {118, 118, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("camera values apply to sspr")
    {
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }
        
        graphics->cls();
        graphics->camera(-100, -100);
        graphics->sspr(0, 0, 8, 8, 10, 10, 8, 8, false, false);
        graphics->camera();


        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {109, 109, 0},
            {110, 110, 0},
            {111, 111, 1},
            {112, 112, 2},
            {113, 113, 3},
            {114, 114, 4},
            {115, 115, 5},
            {116, 116, 6},
            {117, 117, 7},
            {118, 118, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip() resets clip memory to entire screen")
    {
        picoRam.drawState.clip_xb = 28;
        picoRam.drawState.clip_xe = 28;
        picoRam.drawState.clip_yb = 50;
        picoRam.drawState.clip_ye = 50;

        graphics->clip();

        CHECK_EQ(picoRam.drawState.clip_xb, 0);
        CHECK_EQ(picoRam.drawState.clip_xe, 128);
        CHECK_EQ(picoRam.drawState.clip_yb, 0);
        CHECK_EQ(picoRam.drawState.clip_ye, 128);
    }
    SUBCASE("clip({x}, {y}, {w}, {h}) sets clip in memory")
    {
        graphics->clip(50, 75, 25, 25);

        CHECK_EQ(picoRam.drawState.clip_xb, 50);
        CHECK_EQ(picoRam.drawState.clip_xe, 75);
        CHECK_EQ(picoRam.drawState.clip_yb, 75);
        CHECK_EQ(picoRam.drawState.clip_ye, 100);
    }
    SUBCASE("clip() returns prev value")
    {
        picoRam.drawState.clip_xb = 28;
        picoRam.drawState.clip_xe = 29;
        picoRam.drawState.clip_yb = 50;
        picoRam.drawState.clip_ye = 51;

        auto prev = graphics->clip();

        CHECK_EQ(std::get<0>(prev), 28);
        CHECK_EQ(std::get<1>(prev), 29);
        CHECK_EQ(std::get<2>(prev), 50);
        CHECK_EQ(std::get<3>(prev), 51);
    }
    SUBCASE("clip values applies to pset")
    {
       graphics->clip(100, 100, 28, 28);
       graphics->pset(32, 32, 13);
       
       auto result = graphics->pget(32, 32);

       CHECK_EQ(result, 0);
    }
    SUBCASE("clip values apply to diagonal line") 
    {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->line(98, 98, 102, 102, 10);

        std::vector<coloredPoint> expectedPoints = {
            {98, 98, 0},
            {99, 99, 0},
            {100, 100, 10},
            {101, 101, 10},
            {102, 102, 10},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to horizontal line") 
    {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->line(98, 102, 102, 102, 10);

        std::vector<coloredPoint> expectedPoints = {
            {98, 102, 0},
            {99, 102, 0},
            {100, 102, 10},
            {101, 102, 10},
            {102, 102, 10},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to vertical line") 
    {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->line(102, 98, 102, 102, 10);

        std::vector<coloredPoint> expectedPoints = {
            {102, 98,   0},
            {102, 99,   0},
            {102, 100, 10},
            {102, 101, 10},
            {102, 102, 10},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to circ") {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->circ(100, 100, 1, 14);

        std::vector<coloredPoint> expectedPoints = {
            {100, 99, 0},
            {101, 100, 14},
            {100, 101, 14},
            {99, 100, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to circfill") {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->circfill(105, 99, 1, 3);

        std::vector<coloredPoint> expectedPoints = {
            {105, 100, 3},
            {106, 99, 0},
            {105, 98, 0},
            {104, 99, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to rect") {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->rect(98, 98, 101, 101, 4);

        std::vector<coloredPoint> expectedPoints = {
            {98, 98, 0},
            {98, 99, 0},
            {98, 100, 0},
            {98, 101, 0},
            {99, 98, 0},
            {100, 98, 0},
            {101, 98, 0},
            {101, 99, 0},
            {101, 100, 4},
            {101, 101, 4},
            {100, 101, 4},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to rectfill") {
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->rectfill(99, 99, 100, 102, 3);

        std::vector<coloredPoint> expectedPoints = {
            {99, 99, 0},
            {99, 100, 0},
            {99, 101, 0},
            {99, 102, 0},
            {100, 99, 0},
            {100, 100, 3},
            {100, 101, 3},
            {100, 102, 3},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to print") {
        graphics->cls();
        graphics->clip(101, 101, 27, 27);
        graphics->camera(-100, -100);
        graphics->print("t");
        graphics->camera();

        std::vector<coloredPoint> expectedPoints = {
            {100, 100, 0},
            {101, 100, 0},
            {102, 100, 0},
            {101, 101, 6},
            {101, 102, 6},
            {101, 103, 6},
            {101, 104, 6}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to spr") {
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }
        
        graphics->cls();
        graphics->clip(111, 111, 17, 17);
        graphics->spr(0, 110, 110, 1, 1, false, false);

        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {109, 109, 0},
            {110, 110, 0},
            {111, 111, 1},
            {112, 112, 2},
            {113, 113, 3},
            {114, 114, 4},
            {115, 115, 5},
            {116, 116, 6},
            {117, 117, 7},
            {118, 118, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("clip values apply to sspr") {
        for(uint8_t i = 0; i < 16; i++) {
            for(uint8_t j = 0; j < 16; j++) {
                graphics->sset(i, j, i);
            }
        }
        
        graphics->cls();
        graphics->clip(100, 100, 28, 28);
        graphics->sspr(1, 1, 2, 2, 96, 96, 8, 8, false, false);


        //diagonal across sprite
        std::vector<coloredPoint> expectedPoints = {
            {96, 96, 0},
            {97, 97, 0},
            {98, 98, 0},
            {99, 99, 0},
            {100, 100, 2},
            {101, 101, 2},
            {102, 102, 2},
            {103, 103, 2},
            {104, 104, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("mget from upper half of map data (non-shared)"){
        int x = 78;
        int y = 17;
        int idx = y * 128 + x;
        picoRam.mapData[idx] = 242;

        auto result = graphics->mget(x, y);

        CHECK_EQ(242, result);
    }
    SUBCASE("mget from lower half of map data (shared)"){
        int x = 121;
        int y = 57;
        int idx = y * 128 + x;
        picoRam.spriteSheetData[idx] = 71;

        auto result = graphics->mget(x, y);

        CHECK_EQ(71, result);
    }
    SUBCASE("mget out of bounds returns 0"){
        auto result1 = graphics->mget(-1, 16);
        auto result2 = graphics->mget(129, 16);
        auto result3 = graphics->mget(10, -1);
        auto result4 = graphics->mget(10, 67);

        CHECK_EQ(0, result1);
        CHECK_EQ(0, result2);
        CHECK_EQ(0, result3);
        CHECK_EQ(0, result4);
    }
    SUBCASE("mset from upper half of map data (non-shared)"){
        int x = 13;
        int y = 7;
        int idx = y * 128 + x;
        graphics->mset(x, y, 177);

        auto result = picoRam.mapData[idx];

        CHECK_EQ(177, result);
    }
    SUBCASE("mset from lower half of map data (shared)"){
        int x = 12;
        int y = 63;
        int idx = y * 128 + x;
        graphics->mset(x, y, 251);

        auto result = picoRam.spriteSheetData[idx];

        CHECK_EQ(251, result);
    }
    SUBCASE("map with no layer argument draws map cell sprite"){
        for(int i = 0; i < 128; i++) {
            for (int j = 0; j < 32; j++)
            graphics->sset(i, j, 5);
        }
        for(int x = 0; x < 128; x++) {
            for (int y = 0; y < 32; y++) {
                graphics->mset(x, y, 1);
            }
        }

        graphics->map(0,0,0,0,2,2);

        //diagonal across screen- 2x2 cells, 16x16 pixels
        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 5},
            {1, 1, 5},
            {2, 2, 5},
            {3, 3, 5},
            {15, 15, 5},
            {16, 16, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("map with unfilled layer argument doesn't draw"){
        for(int i = 0; i < 128; i++) {
            for (int j = 0; j < 32; j++)
            graphics->sset(i, j, 5);
        }
        for(int x = 0; x < 128; x++) {
            for (int y = 0; y < 32; y++) {
                graphics->mset(x, y, 1);
            }
        }

        graphics->map(0,0,0,0,2,2,5);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 0},
            {1, 1, 0},
            {2, 2, 0},
            {3, 3, 0},
            {15, 15, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("map with filled layer argument doesn't draw"){
        for(int i = 0; i < 128; i++) {
            for (int j = 0; j < 32; j++)
            graphics->sset(i, j, 5);
        }
        for(int x = 0; x < 128; x++) {
            for (int y = 0; y < 32; y++) {
                graphics->mset(x, y, 1);
            }
        }
        uint8_t layer = 3;
        graphics->fset(1, layer);

        graphics->map(0,0,0,0,2,2,layer);

        //diagonal across screen- 2x2 cells, 16x16 pixels
        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 5},
            {1, 1, 5},
            {2, 2, 5},
            {3, 3, 5},
            {15, 15, 5},
            {16, 16, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("pal({c0}, {c1}) remaps draw color") {
        graphics->pal(14, 5, 0);

        auto result = graphics->getDrawPalMappedColor(14);

        CHECK_EQ(5, result);
    }
    SUBCASE("pal({c0}, {c1}, {p}) remaps screen color") {
        graphics->pal(13, 4, 1);

        auto result = graphics->getScreenPalMappedColor(13);

        CHECK_EQ(4, result);
    }
    SUBCASE("pal({c0}, {c1}, 0) returns previous") {
        graphics->pal(14, 2, 0);
        auto result = graphics->pal(14, 8, 0);

        CHECK_EQ(2, result);
    }
    SUBCASE("pal({c0}, {c1}, 1) returns previous") {
        graphics->pal(13, 4, 1);
        auto result = graphics->pal(13, 6, 1);

        CHECK_EQ(4, result);
    }
    SUBCASE("pal({c0}, {c1}, {p}) 17 remapped to 1") {
        graphics->pal(13, 17, 1);

        auto result = graphics->getScreenPalMappedColor(13);

        CHECK_EQ(1, result);
    }
    SUBCASE("pal({c0}, {c1}, {p}) extended palette allowed") {
        graphics->pal(7, 143, 1);

        auto result = graphics->getScreenPalMappedColor(7);

        CHECK_EQ(143, result);
    }
    SUBCASE("palt({c}, {t}) sets transparency value for color") {
        graphics->palt(2, true);

        CHECK(graphics->isColorTransparent(2) == true);
    }
    SUBCASE("palt({c}, {t}) sets transparency value for color") {
        graphics->palt(4, true);
        graphics->palt(4, false);

        CHECK(graphics->isColorTransparent(4) == false);
    }
    SUBCASE("pal() resets all palette changes") {
        graphics->pal(14, 5, 0);
        graphics->pal(13, 4, 1);
        graphics->palt(15, true);

        graphics->pal();

        bool isDefault = true;

        for (uint8_t c = 0; c < 16; c++) {
            isDefault &= graphics->getDrawPalMappedColor(c) == c;
            isDefault &= graphics->isColorTransparent(c) == (c == 0);
            isDefault &= graphics->getScreenPalMappedColor(c) == c;
        }

        CHECK(isDefault);
    }
    SUBCASE("pal changes sprite colors"){
        for(int i = 0; i < 128; i++) {
            for (int j = 0; j < 32; j++)
            graphics->sset(i, j, 5);
        }
        graphics->pal(5, 2, 0);
        graphics->spr(1, 0, 0, 1.0, 1.0, false, false);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 2},
            {1, 1, 2},
            {2, 2, 2},
            {3, 3, 2},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("palt changes sprite transparency"){
        for(int i = 0; i < 128; i++) {
            for (int j = 0; j < 32; j++)
            graphics->sset(i, j, 5);
        }
        graphics->palt(5, true);
        graphics->spr(1, 0, 0, 1.0, 1.0, false, false);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 0},
            {1, 1, 0},
            {2, 2, 0},
            {3, 3, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("pal({c0}, {c1}, {p}) doesn't change default transparency") {
        graphics->pal(0, 0, 0);

        CHECK(graphics->isColorTransparent(0) == true);
    }
    SUBCASE("pal({c0}, {c1}, {p}) doesn't change transparency") {
        graphics->palt(4, true);
        graphics->pal(4, 7, 0);

        CHECK(graphics->isColorTransparent(4) == true);
    }
    SUBCASE("cursor({x}, {y}) sets cursor loc in memory"){
        graphics->cursor(4, 13);

        CHECK_EQ(picoRam.drawState.text_x, 4);
        CHECK_EQ(picoRam.drawState.text_y, 13);
    }
    SUBCASE("cursor({x}, {y}, {c}) sets cursor loc in memory and color"){
        graphics->cursor(42, 99, 3);

        CHECK_EQ(picoRam.drawState.text_x, 42);
        CHECK_EQ(picoRam.drawState.text_y, 99);
        CHECK_EQ(picoRam.drawState.color, 3);
    }
    SUBCASE("cursor() resets cursor but not color"){
        graphics->cursor(42, 99, 3);
        graphics->cursor();

        CHECK_EQ(picoRam.drawState.text_x, 0);
        CHECK_EQ(picoRam.drawState.text_y, 0);
        CHECK_EQ(picoRam.drawState.color, 3);
    }
    SUBCASE("cursor() returns previous values")
    {
        uint8_t x = 33;
        uint8_t y = 120;
        graphics->cursor(x, y);
        auto prev = graphics->cursor(11, 31);

        CHECK_EQ(std::get<0>(prev), x);
        CHECK_EQ(std::get<1>(prev), y);
    }
    SUBCASE("tline draws sprites from map"){
        for(int i = 0; i < 128; i++) {
            for (int j = 0; j < 32; j++)
            graphics->sset(i, j, i%16);
        }
        for(int x = 0; x < 128; x++) {
            for (int y = 0; y < 32; y++) {
                graphics->mset(x, y, (x + y) % 16);
            }
        }

        graphics->tline(27, 34, 34, 41, 3, 4);

        //diagonal across screen- 2x2 cells, 16x16 pixels
        std::vector<coloredPoint> expectedPoints = {
            {27,34,8},
            {28,35,9},
            {29,36,10},
            {30,37,11},
            {31,38,12},
            {32,39,13},
            {33,40,14},
            {34,41,15}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("fillp(pat) sets values in memory"){
        //0011001111001100 - 2x2 checkerboard
        graphics->fillp(0x33cc);

        CHECK_EQ(picoRam.drawState.fillPattern[0], 0xcc);
        CHECK_EQ(picoRam.drawState.fillPattern[1], 0x33);
        CHECK_EQ(picoRam.drawState.fillPatternTransparencyBit, 0);

        CHECK_EQ(picoRam.data[0x5f31], 0xcc);
        CHECK_EQ(picoRam.data[0x5f32], 0x33);
        CHECK_EQ(picoRam.data[0x5f33], 0);
    }
    SUBCASE("fillp(pat) returns previous"){
        graphics->fillp(0x33cc);

        auto prev = graphics->fillp(0x44dd);

        CHECK_EQ(prev.bits(), 0x33cc0000);
    }
    SUBCASE("fill pattern affects rectfill"){
        //0b0101101001011010 - 1x1 checkerboard
        graphics->fillp(0x5A5A);
        graphics->cls(8);

        graphics->rectfill(0, 0, 4, 4, 10);

        //black and yellow 4x4 checkerboard, with pink elsewhere
        std::vector<coloredPoint> expectedPoints = {
            { 0, 0,10},
            { 1, 0, 0},
            { 2, 0,10},
            { 3, 0, 0},
            { 4, 0,10},
            { 5, 0, 8},
            { 0, 1, 0},
            { 1, 1,10},
            { 2, 1, 0},
            { 3, 1,10},
            { 4, 1, 0},
            { 5, 1, 8},
            { 0, 2,10},
            { 1, 2, 0},
            { 2, 2,10},
            { 3, 2, 0},
            { 4, 2,10},
            { 5, 2, 8},
            { 0, 3, 0},
            { 1, 3,10},
            { 2, 3, 0},
            { 3, 3,10},
            { 4, 3, 0},
            { 5, 3, 8},
            { 0, 4,10},
            { 1, 4, 0},
            { 2, 4,10},
            { 3, 4, 0},
            { 4, 4,10},
            { 5, 4, 8},
            { 0, 5, 8},
            { 1, 5, 8},
            { 2, 5, 8},
            { 3, 5, 8},
            { 4, 5, 8},
            { 5, 5, 8},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("fill pattern with transparency"){
        //0b0101101001011010.1 - 1x1 checkerboard with transparency
        graphics->fillp(23130.5);
        graphics->cls(3);

        graphics->rectfill(0, 0, 2, 2, 11);
        
        //3x3 checkerboard, with alt set to transparent
        std::vector<coloredPoint> expectedPoints = {
            { 0, 0,11},
            { 1, 0, 3},
            { 2, 0,11},
            { 3, 0, 3},
            { 0, 1, 3},
            { 1, 1,11},
            { 2, 1, 3},
            { 3, 1, 3},
            { 0, 2,11},
            { 1, 2, 3},
            { 2, 2,11},
            { 3, 2, 3},
            { 0, 3, 3},
            { 1, 3, 3},
            { 2, 3, 3},
            { 3, 3, 3}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("fill pattern oriented correctly"){
        //311 = 0b0000000100110111
        //0000
        //0001
        //0011
        //0111
        graphics->cls(0);
        graphics->fillp(311);

        //e8: light pink alt color, pink normal color
        graphics->rectfill(0, 0, 3, 3, 0xe8);

        //black and yellow 4x4 checkerboard, with pink elsewhere
        std::vector<coloredPoint> expectedPoints = {
            { 0, 0, 8},
            { 1, 0, 8},
            { 2, 0, 8},
            { 3, 0, 8},
            { 4, 0, 0},
            { 0, 1, 8},
            { 1, 1, 8},
            { 2, 1, 8},
            { 3, 1,14},
            { 4, 1, 0},
            { 0, 2, 8},
            { 1, 2, 8},
            { 2, 2,14},
            { 3, 2,14},
            { 4, 2, 0},
            { 0, 3, 8},
            { 1, 3,14},
            { 2, 3,14},
            { 3, 3,14},
            { 4, 3, 0},
            { 0, 4, 0},
            { 1, 4, 0},
            { 2, 4, 0},
            { 3, 4, 0},
            { 4, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    //TODO: allow this shorthand to work
    /*
    SUBCASE("fill pattern shorthand works"){
        graphics->cls(0);
        picoRam.drawState.colorSettingFlag = 1;
        //0x104e.abcd
        graphics->rectfill(0, 0, 3, 3, 0x104eabcd);

        CHECK_EQ(picoRam.drawState.fillPattern[0], 0xcd);
        CHECK_EQ(picoRam.drawState.fillPattern[1], 0xab);
        CHECK_EQ(picoRam.drawState.fillPatternTransparencyBit, 0);

        //black and yellow 4x4 checkerboard, with pink elsewhere
        std::vector<coloredPoint> expectedPoints = {
            { 0, 0, 4},
            { 1, 0,14},
            { 2, 0, 4},
            { 3, 0,14},
            { 4, 0, 0},
            { 0, 1, 4},
            { 1, 1,14},
            { 2, 1, 4},
            { 3, 1, 4},
            { 4, 1, 0},
            { 0, 2, 4},
            { 1, 2, 4},
            { 2, 2,14},
            { 3, 2,14},
            { 4, 2, 0},
            { 0, 3, 4},
            { 1, 3, 4},
            { 2, 3,14},
            { 3, 3, 4},
            { 4, 3, 0},
            { 0, 4, 0},
            { 1, 4, 0},
            { 2, 4, 0},
            { 3, 4, 0},
            { 4, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    */
    

    //general teardown
    delete graphics;
}
#include <string>
#include <vector>

#include "doctest.h"

#include "../source/vm.h"
#include "../source/hostVmShared.h"
#include "../source/host.h"
#include "../source/printHelper.h"
#include "../source/Audio.h"
#include "../source/graphics.h"
#include "stubhost.h"

#include "../source/fontdata.h"


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


TEST_CASE("Print helper functions") {
    StubHost* stubHost = new StubHost();
    PicoRam* memory = new PicoRam();
    memory->Reset();
    Graphics* graphics = new Graphics(get_font_data(), memory);
    Input* input = new Input(memory);
    Audio* audio = new Audio(memory);

    Vm* vm = new Vm(stubHost, memory, graphics, input, audio);

    initPrintHelper(memory, graphics, vm, audio);

    SUBCASE("print({str}) uses current color, ignoring transparency") {
        graphics->cls();
        graphics->color(2);
        graphics->palt(2, true);

        print("t");

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
        memory->drawState.text_x = 15;
        memory->drawState.text_y = 98;

        print("t");

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
        memory->drawState.text_x = 15;
        memory->drawState.text_y = 110;

        print("doesnt matter");

        CHECK(memory->drawState.text_y == 116);
    }
    SUBCASE("print({str}, {x}, {y}) updates text location ") {
        graphics->cls();
        memory->drawState.text_x = 3;
        memory->drawState.text_y = 4;

        print("doesnt matter", 42, 99);

        CHECK(memory->drawState.text_x == 42);
        CHECK(memory->drawState.text_y == 105);
    }
    SUBCASE("print({str}, {x}, {y}, {c}) updates text location and color") {
        graphics->cls();
        memory->drawState.text_x = 3;
        memory->drawState.text_y = 4;
        memory->drawState.color = 10;

        print("doesnt matter", 16, 18, 14);
        
        CHECK(memory->drawState.text_x == 16);
        CHECK(memory->drawState.text_y == 24);
        CHECK(memory->drawState.color == 14);
    }
    SUBCASE("print({str}) uses pal mapped color") {
        graphics->cls();
        graphics->color(2);
        graphics->pal(2, 12, 0);

        print("t");

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
    SUBCASE("camera values apply to print")
    {
        graphics->cls();
        graphics->camera(-100, -100);
        print("t");
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
    SUBCASE("clip values apply to print") {
        graphics->cls();
        graphics->clip(101, 101, 27, 27);
        graphics->camera(-100, -100);
        print("t");
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
    SUBCASE("p8scii tab character advances to next x multiple of 16 (default tab size)") {
        graphics->cls();

        print("a\t:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {17, 0, 0},
            {17, 1, 6},
            {17, 2, 0},
            {17, 3, 6},
            {17, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii backspace character moves cursor backward 4 pixels (doesn't erase)") {
        graphics->cls();

        print("i\b-", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 6}, {1, 0, 6}, {2, 0, 6},
            {0, 1, 0}, {1, 1, 6}, {2, 1, 0},
            {0, 2, 6}, {1, 2, 6}, {2, 2, 6},
            {0, 3, 0}, {1, 3, 6}, {2, 3, 0},
            {0, 4, 6}, {1, 4, 6}, {2, 4, 6},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii carriage return character moves cursor back to current cursor x(doesn't erase)") {
        graphics->cls();

        print("i\r-", 10, 0);

        std::vector<coloredPoint> expectedPoints = {
            {10, 0, 6}, {11, 0, 6}, {12, 0, 6},
            {10, 1, 0}, {11, 1, 6}, {12, 1, 0},
            {10, 2, 6}, {11, 2, 6}, {12, 2, 6},
            {10, 3, 0}, {11, 3, 6}, {12, 3, 0},
            {10, 4, 6}, {11, 4, 6}, {12, 4, 6},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii repeat character (\\*) draws character x number of times") {
        graphics->cls();

        print("\x01""5:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {17, 0, 0},
            {17, 1, 6},
            {17, 2, 0},
            {17, 3, 6},
            {17, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii bg color character (\\#) changes bg color") {
        graphics->cls();

        print("\x02""1:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 1},
            {1, 1, 6},
            {1, 2, 1},
            {1, 3, 6},
            {1, 4, 1}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii fg color character (\\f) changes bg color") {
        graphics->cls();

        print("\x0c""2:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 2},
            {1, 2, 0},
            {1, 3, 2},
            {1, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii bg color character (\\#) changes bg color using hex") {
        graphics->cls();

        print("\x02""c:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 12},
            {1, 1, 6},
            {1, 2, 12},
            {1, 3, 6},
            {1, 4, 12}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii horizontal move character (\\-) changes x location") {
        graphics->cls();

        print("\x03""a:", 10, 0);

        std::vector<coloredPoint> expectedPoints = {
            {5, 0, 0},
            {5, 1, 6},
            {5, 2, 0},
            {5, 3, 6},
            {5, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii vertical move character (\\|) changes y location") {
        graphics->cls();

        print("\x05""ab:", 6, 5);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 6},
            {1, 2, 0},
            {1, 3, 6},
            {1, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }


    delete stubHost;
    delete graphics;
    delete input;
    delete audio;

    delete vm;

    delete memory;
}

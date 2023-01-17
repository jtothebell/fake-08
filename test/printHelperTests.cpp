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

#include "testHelpers.h"



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
    SUBCASE("print({str}, {x}, {y}, {c}) updates text location correct in multiline situation") {
        graphics->cls();
        memory->drawState.text_x = 3;
        memory->drawState.text_y = 4;
        memory->drawState.color = 10;

        print("doesnt\nmatter\nwell\nkinda\ndoes", 19, 18, 14);
        
        CHECK(memory->drawState.text_x == 19);
        CHECK(memory->drawState.text_y == 48);
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
    SUBCASE("p8scii special control code clear screen(\\^c) ") {
        graphics->cls(2);

        print("88888");

        print("\x06""c3", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 3},
            {1, 1, 3},
            {1, 2, 3},
            {1, 3, 3},
            {1, 4, 3}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code home the cursor(\\^g) ") {
        graphics->cls();

        print("\n\n\nstuff\x06""g:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 6},
            {1, 2, 0},
            {1, 3, 6},
            {1, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code update cursor home(\\^h) ") {
        graphics->cls();

        print("\n\n\n\x06""h\n\n\nmorestuff\x06""g:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 18, 0},
            {1, 19, 6},
            {1, 20, 0},
            {1, 21, 6},
            {1, 22, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code move cursor(\\^j) ") {
        graphics->cls();

        // coordinates x=40 ("a" = 10, 10 * 4 = 40), y=48 ("c" = 12, 12 * 4 = 48)
        print("\x06""jac:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {41, 48, 0},
            {41, 49, 6},
            {41, 50, 0},
            {41, 51, 6},
            {41, 52, 0}
        };

        checkPoints(graphics, expectedPoints);
    }

       SUBCASE("p8scii special control code move cursor(\\^j) and update home and color test") {
        graphics->cls();
    //"\^j87\-f\|a\^h\f7:\^je8other\n:"
    /*
--\^j87- move cursor to 32,28
 --\-f- move cursor horizontally -1 (31,28)
 --\|a- move cursor vertically -6 (31,22)
 --\^h --set cursor home to be current location (31,22)
 --\f7 change color to 7 (white)
 --: print the ":" character
 --\^je8- move cursor to 56,32
 --print "other\n" followed by a new line
 --print one more ":"
    */
        // coordinates x=40 ("a" = 10, 10 * 4 = 40), y=48 ("c" = 12, 12 * 4 = 48)
        print("\x06""j87\x03""f\x04""a\x06""h\x0c""7:\x06""je8:\n:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {32, 22, 0},
            {32, 23, 7},
            {32, 24, 0},
            {32, 25, 7},
            {32, 26, 0},

            {32, 38, 0},
            {32, 39, 7},
            {32, 40, 0},
            {32, 41, 7},
            {32, 42, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code tab stop width(\\^s) ") {
        graphics->cls();

        print("\x06""sc \t:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {49, 0, 0},
            {49, 1, 6},
            {49, 2, 0},
            {49, 3, 6},
            {49, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for rhs wrap(\\^r) ") {
        graphics->cls();

        print("\x06""rsthis is a long string that should wrap somewhere", 0, 0);

        //the "h" in "should" is the first character to wrap
        std::vector<coloredPoint> expectedPoints = {
            {0, 6, 6},
            {0, 7, 6},
            {0, 8, 6},
            {0, 9, 6},
            {0, 10, 6},
            {0, 11, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for char width(\\^x) ") {
        graphics->cls();

        print("\x06""x7 :", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {8, 0, 0},
            {8, 1, 6},
            {8, 2, 0},
            {8, 3, 6},
            {8, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for char width(\\^x) affects bg color ") {
        graphics->cls();

        print("\x06""xz\x06""j00\x02""9 ", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {30, 0, 9},
            {31, 1, 9},
            {32, 2, 9},
            {33, 3, 9},
            {34, 4, 9},
            {35, 5, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for wide character(\\^w) ") {
        graphics->cls();

        print("\x06""w::", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {2, 0, 0}, {3, 0, 0},    {10, 0, 0}, {11, 0, 0},
            {2, 1, 6}, {3, 1, 6},    {10, 1, 6}, {11, 1, 6},
            {2, 2, 0}, {3, 2, 0},    {10, 2, 0}, {11, 2, 0},
            {2, 3, 6}, {3, 3, 6},    {10, 3, 6}, {11, 3, 6},
            {2, 4, 0}, {3, 4, 0},    {10, 4, 0}, {11, 4, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for char height(\\^t) ") {
        graphics->cls();

        print("\x06""t:\n:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 0},
            {1, 2, 6},
            {1, 3, 6},
            {1, 4, 0},
            {1, 5, 0},
            {1, 6, 6},
            {1, 7, 6},
            {1, 8, 0},
            {1, 9, 0},

            {1, 10, 0},
            {1, 11, 0},

            {1, 12, 0},
            {1, 13, 0},
            {1, 14, 6},
            {1, 15, 6},
            {1, 16, 0},
            {1, 17, 0},
            {1, 18, 6},
            {1, 19, 6},
            {1, 20, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for wide character with stripey option(\\^w\\^=) ") {
        graphics->cls();

        print("\x06""w""\x06""=:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {2, 0, 0}, {3, 0, 0},
            {2, 1, 6}, {3, 1, 0},
            {2, 2, 0}, {3, 2, 0},
            {2, 3, 6}, {3, 3, 0},
            {2, 4, 0}, {3, 4, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for char height with stripey option(\\^t\\^=) ") {
        graphics->cls();

        print("\x06""t""\x06""=:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 0},
            {1, 2, 6},
            {1, 3, 0},
            {1, 4, 0},
            {1, 5, 0},
            {1, 6, 6},
            {1, 7, 0},
            {1, 8, 0},
            {1, 9, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for pinball option(\\^p) ") {
        graphics->cls();

        print("\x06""p:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {2, 0, 0}, {3, 0, 0},
            {2, 1, 0}, {3, 1, 0},
            {2, 2, 6}, {3, 2, 0},
            {2, 3, 0}, {3, 3, 0},
            {2, 4, 0}, {3, 4, 0},
            {2, 5, 0}, {3, 5, 0},
            {2, 6, 6}, {3, 6, 0},
            {2, 7, 0}, {3, 7, 0},
            {2, 8, 0}, {3, 8, 0},
            {2, 9, 0}, {3, 9, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code to turn off options(\\^-) ") {
        graphics->cls();

        print("\x06""w""\x06""t8" "\x06""-w""\x06""-t:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {9, 0, 0},
            {9, 1, 6},
            {9, 2, 0},
            {9, 3, 6},
            {9, 4, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code to turn off options uses biggest line height(\\^-) ") {
        graphics->cls();

        print("\x06""w""\x06""t8" "\x06""-w""\x06""-t:", 0, 0);
        print(":");

        std::vector<coloredPoint> expectedPoints = {
            {1, 12, 0},
            {1, 13, 6},
            {1, 14, 0},
            {1, 15, 6},
            {1, 16, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for one off character(\\^:) (colored)") {
        graphics->cls();

        print("\x0c""2\x06"":447cb67c3e7f0106", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 0},
            {1, 0, 0},
            {2, 0, 2},
            {3, 0, 0},
            {4, 0, 0},
            {5, 0, 0},
            {6, 0, 2},
            {7, 0, 0},
            {0, 1, 0},
            {1, 1, 0},
            {2, 1, 2},
            {3, 1, 2},
            {4, 1, 2},
            {5, 1, 2},
            {6, 1, 2},
            {7, 1, 0},
            {0, 2, 0},
            {1, 2, 2},
            {2, 2, 2},
            {3, 2, 0},
            {4, 2, 2},
            {5, 2, 2},
            {6, 2, 0},
            {7, 2, 2},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for one off character(\\^:) (pinballed with bg)") {
        graphics->cls();

        print("\x02""4\x06""p\x06"":447cb67c3e7f0106", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 4},
            {1, 0, 4},
            {2, 0, 4},
            {3, 0, 4},
            {4, 0, 6},
            {5, 0, 4},
            {6, 0, 4},
            {7, 0, 4},
            {8, 0, 4},
            {9, 0, 4},
            {10, 0, 4},
            {11, 0, 4},
            {12, 0, 6},
            {13, 0, 4},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("Poke default print mode but not turned on") {
        graphics->cls();

        //not turned on
        //                   wide  tall  dotty/stripey
        vm->vm_poke(0x5f58, (0x4 | 0x8 | 0x40));
        print(":", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 6},
            {1, 2, 0},
            {1, 3, 6},
            {1, 4, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("Poke default print mode (pinball)") {
        graphics->cls();

        //                  on    wide  tall  dotty/stripey
        vm->vm_poke(0x5f58, (0x1 | 0x4 | 0x8 | 0x40));
        print(":", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {2, 0, 0}, {3, 0, 0},
            {2, 1, 0}, {3, 1, 0},
            {2, 2, 6}, {3, 2, 0},
            {2, 3, 0}, {3, 3, 0},
            {2, 4, 0}, {3, 4, 0},
            {2, 5, 0}, {3, 5, 0},
            {2, 6, 6}, {3, 6, 0},
            {2, 7, 0}, {3, 7, 0},
            {2, 8, 0}, {3, 8, 0},
            {2, 9, 0}, {3, 9, 0}
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for char width(\\^x) and char height (\\^y) limit rendering ") {
        graphics->cls();

        print("\x06""x2\x06""y3a", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 6},
            {1, 0, 6},
            {2, 0, 0},
            {3, 0, 0},

            {0, 1, 6},
            {1, 1, 0},
            {2, 1, 0},
            {3, 1, 0},

            {0, 2, 6},
            {1, 2, 6},
            {2, 2, 0},
            {3, 2, 0},

            {0, 3, 0},
            {1, 3, 0},
            {2, 3, 0},
            {3, 3, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for char height (\\^y) sets line height correctly when lower") {
        graphics->cls();

        print("\x06""y3a", 0, 0);

        CHECK_EQ(memory->drawState.text_y, 3);
    }
    SUBCASE("p8scii special control code for char height (\\^y) sets line height correctly when higher") {
        graphics->cls();

        print("\x06""y9a", 0, 0);

        CHECK_EQ(memory->drawState.text_y, 9);
    }
    SUBCASE("p8scii audio control codes not printed(\\a)") {
        graphics->cls();

        print("\x07""aceg :", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {1, 0, 0},
            {1, 1, 6},
            {1, 2, 0},
            {1, 3, 6},
            {1, 4, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii control code for decorating prev char (\\v)") {
        graphics->cls();

        print("\n:\x0b""b:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {2, 0, 0},
            {2, 1, 6},
            {2, 2, 0},
            {2, 3, 6},
            {2, 4, 0},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii custom font control code (\\014 on and \\015 off)") {
        graphics->cls();
        memory->data[0x5600]= 8; //width
        memory->data[0x5601]= 8; //width for chars > 127
        memory->data[0x5602]= 8; //height
        memory->data[0x5603]= 0;
        memory->data[0x5604]= 0;

        //char 16
        memory->data[0x5680]= 1;    //#_______
        memory->data[0x5681] = 3;   //##______
        memory->data[0x5682] = 7;   //###_____
        memory->data[0x5683] = 15;  //####____
        memory->data[0x5684] = 31;  //#####___
        memory->data[0x5685] = 63;  //######__
        memory->data[0x5686] = 127; //#######_
        memory->data[0x5687] = 255; //########

        //print char 16 with custom font
        print("\x0e""\x10""\x0f""", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 6}, {1, 0, 0},
            {0, 1, 6}, {1, 1, 6}, {2, 1, 0},
            {0, 2, 6}, {1, 2, 6}, {2, 2, 6}, {3, 2, 0},
            {0, 3, 6}, {1, 3, 6}, {2, 3, 6}, {3, 3, 6}, {4, 3, 0},
            {0, 4, 6}, {1, 4, 6}, {2, 4, 6}, {3, 4, 6}, {4, 4, 6}, {5, 4, 0},
            {0, 5, 6}, {1, 5, 6}, {2, 5, 6}, {3, 5, 6}, {4, 5, 6}, {5, 5, 6}, {6, 5, 0},
            {0, 6, 6}, {1, 6, 6}, {2, 6, 6}, {3, 6, 6}, {4, 6, 6}, {5, 6, 6}, {6, 6, 6}, {7, 6, 0},
            {0, 7, 6}, {1, 7, 6}, {2, 7, 6}, {3, 7, 6}, {4, 7, 6}, {5, 7, 6}, {6, 7, 6}, {7, 7, 6}, {7, 8, 0},
            {0, 8, 0}

        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for inverting colors(\\^i) ") {
        graphics->cls(2);

        print("\x06""i:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 6}, {1, 0, 6}, {2, 0, 6},
            {0, 1, 6}, {1, 1, 2}, {2, 1, 6},
            {0, 2, 6}, {1, 2, 6}, {2, 2, 6},
            {0, 3, 6}, {1, 3, 2}, {2, 3, 6},
            {0, 4, 6}, {1, 4, 6}, {2, 4, 6},
        };

        checkPoints(graphics, expectedPoints);
    }
    SUBCASE("p8scii special control code for solid background(\\^#) ") {
        graphics->cls(2);

        print("\x06""#:", 0, 0);

        std::vector<coloredPoint> expectedPoints = {
            {0, 0, 0}, {1, 0, 0}, {2, 0, 0},
            {0, 1, 0}, {1, 1, 6}, {2, 1, 0},
            {0, 2, 0}, {1, 2, 0}, {2, 2, 0},
            {0, 3, 0}, {1, 3, 6}, {2, 3, 0},
            {0, 4, 0}, {1, 4, 0}, {2, 4, 0},
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

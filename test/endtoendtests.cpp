#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "doctest.h"
#include "../libs/lodepng/lodepng.h"

#include "../source/vm.h"
#include "../source/host.h"
#include "../source/hostVmShared.h"
#include "../source/nibblehelpers.h"

bool verifyScreenshot(Vm* vm, std::string screenshotFilename) {
    std::vector<unsigned char> png;
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, screenshotFilename);
    if(!error) {
        error = lodepng::decode(image, width, height, png);
    } 
    if (error) {
        CHECK_MESSAGE(error == 0, "Unable to decode screenshot png");
        return false;
    }

    bool pixelsMatch = true;

    uint8_t* picoFb = vm->GetPicoInteralFb();
	uint8_t* screenPaletteMap = vm->GetScreenPaletteMap();
	Color* paletteColors = vm->GetPaletteColors();

    size_t imageBytes = image.size();

    for(size_t i = 0; i < imageBytes; i += 4) {
        //get argb values
        uint8_t r = image[i];
        uint8_t g = image[i + 1];
        uint8_t b = image[i + 2];
        //ignore alpha
        //uint8_t a = image[i + 3];

        int pixIdx = i / 4;
        int x = pixIdx % 128;
        int y = pixIdx / 128;
        uint8_t c = getPixelNibble(x, y, picoFb);
        Color col = paletteColors[screenPaletteMap[c]];

        //black varies by pico 8 version. allow for either so I don't have to
        //retake all the screenshots
        bool pixelIsBlack = (r == 0 && g == 0 && b == 0) || (r == 2 && g == 4 && b == 8);
        bool pixelMatches = 
            (pixelIsBlack && c == 0) || 
            (r == col.Red && g == col.Green && b == col.Blue);

        /*
        if (!pixelMatches) {
            printf("Non-matching pixel at idx %d (%d,%d): rgb: %d,%d,%d\n", pixIdx, x, y, r, g, b);

            printf("fake 08 color is %d rgb: %d,%d,%d\n", c, col.Red, col.Green, col.Blue);
        }
        */

        pixelsMatch &= pixelMatches;
    }

    return pixelsMatch;

}

TEST_CASE("Loading and running carts") {
    Host* host = new Host();
    Vm* vm = new Vm(host);

    SUBCASE("Load simple cart"){
        vm->LoadCart("cartparsetest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("Frame count updated with each UpdateAndDrawCall()"){
            vm->UpdateAndDraw();
            vm->UpdateAndDraw();
            vm->UpdateAndDraw();

            CHECK(vm->GetFrameCount() == 3);
        }
        SUBCASE("check lua state"){
            bool globalVarLoaded = vm->ExecuteLua(
                "function globalVarTest()\n"
                " return a == 1\n"
                "end\n",
                "globalVarTest");

            CHECK(globalVarLoaded);
        }

        vm->CloseCart();
    }
    SUBCASE("pset 0,0 test cart renders correctly"){
        vm->LoadCart("pset00-test.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/pset00-test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pset 3 pix top left test cart renders correctly"){
        vm->LoadCart("pset3pix.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/pset3pix_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pset all pixels test cart renders correctly"){
        vm->LoadCart("psetall.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/psetall_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Clip test cart renders correctly"){
        vm->LoadCart("cliptest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/cliptest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Memory function test cart"){
        vm->LoadCart("memorytest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/memorytest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Cart data function test cart"){
        vm->LoadCart("cartdatatest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/cartdatatest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("tonum test cart"){
        vm->LoadCart("tonumtest2.p8");

        SUBCASE("can parse positive int"){
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r1test()\n"
                " return r1 == 12345\n"
                "end\n",
                "r1test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("can parse negative decimal") {
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r2test()\n"
                " return r2 == -12345.67\n"
                "end\n",
                "r2test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("can parse hex") {
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r3test()\n"
                " return r3 == 15\n"
                "end\n",
                "r3test");

            CHECK(parsedCorrectly);
        }
        
       SUBCASE("can parse binary literal") {
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r5test()\n"
                " return r5 == 9\n"
                "end\n",
                "r5test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("can parse large positive number") {
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r6test()\n"
                " return r6 == 32767\n"
                "end\n",
                "r6test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("unparseable string returns nil") {
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r8test()\n"
                " return r8 == nil\n"
                "end\n",
                "r8test");

            CHECK(parsedCorrectly);
        }
        
        //currently failing
        SUBCASE("can parse hex with decimal") {
            vm->UpdateAndDraw();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r4test()\n"
                " return r4 == 15.6709\n"
                "end\n",
                "r4test");

            
            CHECK(parsedCorrectly);
        }
        //SUBCASE("too large int overflows") {
        //    vm->UpdateAndDraw();

        //    bool parsedCorrectly = vm->ExecuteLua(
        //        "function r7test()\n"
        //        " return r7 == -32768\n"
        //        "end\n",
        //        "r7test");

        //    CHECK(parsedCorrectly);
        ///}

        vm->CloseCart();
    }
    SUBCASE("Simple Arithmetic Cart"){
        vm->LoadCart("arithmetictest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("addition works"){
            vm->UpdateAndDraw();

            bool additionWorks = vm->ExecuteLua(
                "function additiontest()\n"
                " return frames == 1\n"
                "end\n",
                "additiontest");

            CHECK(additionWorks);
        }
        SUBCASE("multiplication works"){
            vm->UpdateAndDraw();

            bool multiplicationWorks = vm->ExecuteLua(
                "function multiplicationtest()\n"
                " return result == 10\n"
                "end\n",
                "multiplicationtest");

            CHECK(multiplicationWorks);
        }
        SUBCASE("division works"){
            vm->UpdateAndDraw();

            bool divisionWorks = vm->ExecuteLua(
                "function divisiontest()\n"
                " return result2 == 0.2\n"
                "end\n",
                "divisiontest");

            CHECK(divisionWorks);
        }
        SUBCASE("subtraction works"){
            vm->UpdateAndDraw();

            bool subtractionWorks = vm->ExecuteLua(
                "function subtractiontest()\n"
                " return result3 == 3.6\n"
                "end\n",
                "subtractiontest");

            CHECK(subtractionWorks);
        }
        SUBCASE("rnd returns between 0 and 1, and can add"){
            vm->UpdateAndDraw();

            bool rndWorks = vm->ExecuteLua(
                "function rndtest()\n"
                " return result4 > 0 and result4 < 1\n"
                "end\n",
                "rndtest");

            CHECK(rndWorks);
        }
        SUBCASE("rnd returns between 0 and 1 without calling srand first"){
            vm->UpdateAndDraw();

            bool rndWorks = vm->ExecuteLua(
                "function firstrndtest()\n"
                " return firstrand > 0 and firstrand < 1\n"
                "end\n",
                "firstrndtest");

            CHECK(rndWorks);
        }
        SUBCASE("# returns count of array"){
            vm->UpdateAndDraw();

            bool rndWorks = vm->ExecuteLua(
                "function counttest()\n"
                " return tblcount == 4\n"
                "end\n",
                "counttest");

            CHECK(rndWorks);
        }
        SUBCASE("ceil works"){
            vm->UpdateAndDraw();

            bool ceilWorks = vm->ExecuteLua(
                "function ceiltest()\n"
                " return ceilcount == 1\n"
                "end\n",
                "ceiltest");

            CHECK(ceilWorks);
        }

        vm->CloseCart();
    }

    SUBCASE("api loaded with cart load") {

        vm->LoadCart("cartparsetest.p8");

        SUBCASE("all api functions exist"){
            vector<string> apiFunctions {
                "time", "t", "sub", "chr", "ord", "tostr", "tonum", 
                "add", "del", "deli", "clip", "color", "pal", "palt",
                "fillp", "pget", "pset", "sget", "sset", "fget", 
                "fset", "circ", "circfill", "rect", "rectfill", "oval",
                "ovalfill", "line", "spr", "sspr", "mget", "mset", 
                "tline", "peek", "poke", "peek2", "poke2", "peek4",
                "poke4", "memcpy", "memset", "max", "min", "mid", "flr", 
                "ceil", "cos", "sin", "atan2", "rnd", "srand", "band",
                "bor", "bxor", "bnot", "shl", "shr", "lshr", "rotl", "rotr",
                "mapdraw", "extcmd"
            };

            string missing = "";
            
            for (auto apiFunction : apiFunctions) {
                std::stringstream ss;
                ss << "function globalfunctest()\n  return " << apiFunction << " ~= nil\n  end\n";

                bool functionExists = vm->ExecuteLua(ss.str(), "globalfunctest");

                if (!functionExists) {
                    missing = missing + ", " + apiFunction;
                }
            }

            CHECK_MESSAGE(missing.length() == 0, missing);
        }

        vm->CloseCart();
    }
    SUBCASE("calling reload from init doesn't crash") {
        vm->LoadCart("reloadininit.p8");
        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
    }
    SUBCASE("rnd with table argument works") {
        vm->LoadCart("tablerndtest.p8");
        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
    }
    SUBCASE("Fill pattern test cart"){
        vm->LoadCart("fillptest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/fillptest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Peek4/poke4 test cart"){
        vm->LoadCart("peek4test.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            //first frame doesn't have uniform colors
            vm->UpdateAndDraw();
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/peek4test_f02.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pal (with table) test cart"){
        vm->LoadCart("paltabletest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/paltabletest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pairs() with nil arg test"){
        vm->LoadCart("nilpairstest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/nilpairstest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("split() test"){
        vm->LoadCart("splittest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/splittest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("General use memory persists across cart loads"){
        vm->LoadCart("cartparsetest.p8");
        auto origFirstByte = vm->vm_peek(0);
        vm->vm_poke(0x0000, 12);
        vm->vm_poke(0x42ff, 92);

        vm->vm_poke(0x4300, 33);
        vm->vm_poke(0x55ff, 129);

        vm->vm_poke(0x5600, 71);
        vm->vm_poke(0x7fff, 223);
        vm->LoadCart("cartparsetest.p8");

        CHECK_EQ(vm->vm_peek(0x0000), origFirstByte);
        CHECK_EQ(vm->vm_peek(0x42ff), 0);

        CHECK_EQ(vm->vm_peek(0x4300), 33);
        CHECK_EQ(vm->vm_peek(0x55ff), 129);

        CHECK_EQ(vm->vm_peek(0x5600), 0);
        CHECK_EQ(vm->vm_peek(0x7fff), 0);


        vm->CloseCart();
    }
    SUBCASE("#include test"){
        vm->LoadCart("includetest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->UpdateAndDraw();

            CHECK(verifyScreenshot(vm, "carts/screenshots/includetest_f01.png"));
        }

        vm->CloseCart();
    }
    
    delete vm;
    delete host;
}
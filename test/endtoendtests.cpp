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

bool verifyScreenshot(Vm* vm, Host* host, std::string screenshotFilename) {
    std::vector<unsigned char> png;
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //load and decode
    unsigned error = lodepng::load_file(png, screenshotFilename);
    if(!error) {
        error = lodepng::decode(image, width, height, png);
    } 
    if (error) {
        CHECK_MESSAGE(error == 0, "Unable to decode screenshot png %s", screenshotFilename.c_str());
        return false;
    }

    bool pixelsMatch = true;

    uint8_t* picoFb = vm->GetPicoInteralFb();
	uint8_t* screenPaletteMap = vm->GetScreenPaletteMap();
    host->setUpPaletteColors();
	Color* paletteColors = host->GetPaletteColors();

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
        Color col = paletteColors[screenPaletteMap[c] & 0x8f];

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

    // If pixels don't match, save the actual screen for comparison
    // Uncomment this block for debugging screenshot mismatches
    /*
    if (!pixelsMatch) {
        std::vector<unsigned char> actualImage;
        actualImage.resize(128 * 128 * 4);
        
        for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 128; x++) {
                uint8_t c = getPixelNibble(x, y, picoFb);
                Color col = paletteColors[screenPaletteMap[c] & 0x8f];
                size_t idx = (y * 128 + x) * 4;
                actualImage[idx] = col.Red;
                actualImage[idx + 1] = col.Green;
                actualImage[idx + 2] = col.Blue;
                actualImage[idx + 3] = 255;
            }
        }
        
        // Generate output filename based on input
        std::string actualFilename = screenshotFilename;
        size_t dotPos = actualFilename.rfind('.');
        if (dotPos != std::string::npos) {
            actualFilename = actualFilename.substr(0, dotPos) + "_actual.png";
        } else {
            actualFilename += "_actual.png";
        }
        
        unsigned encodeError = lodepng::encode(actualFilename, actualImage, 128, 128);
        if (encodeError) {
            printf("Failed to save actual screenshot to %s\n", actualFilename.c_str());
        } else {
            printf("Saved actual screenshot to %s\n", actualFilename.c_str());
        }
    }
    */

    return pixelsMatch;

}

/*
#include <filesystem>
namespace fs = std::filesystem;

std::vector<std::string> get_cart_files_in_dir(std::string directory){
    std::vector<std::string> files;
    for (const auto & entry : fs::directory_iterator(directory)) {
        std::string path = entry.path().string();
        if (isCartFile(path)) {
            files.push_back(path.substr(path.find_last_of("/\\") + 1));
        }
    }

    return files;
}
*/

TEST_CASE("Loading and running carts") {
    Host* host = new Host();
    Vm* vm = new Vm(host);

    /*
    SUBCASE("test carts") {
        // char cwd[PATH_MAX];
        // if (getcwd(cwd, sizeof(cwd)) != NULL) {
        //     printf("Current working dir: %s\n", cwd);
        // }
        auto screenshots = get_cart_files_in_dir("carts/screenshots");

        for (auto screenshot : screenshots) {
            if (screenshot.length() < 8) {
                continue;
            }
            auto last8Chars = screenshot.substr(screenshot.length() - 8);
            if (last8Chars != "_f01.png") {
                continue;
            }
            auto cartfile = screenshot.substr(0, screenshot.length() - 8) + ".p8";
            vm->LoadCart(cartfile, false);
            vm->vm_reset();

            SUBCASE("No error reported"){
                CHECK(vm->GetBiosError() == "");
            }
            if (getFileExtension(cartfile) == ".p8") {
                SUBCASE(cartfile.c_str()){
                    vm->Step();

                    std::stringstream ss;
                    ss << "carts/screenshots/" << screenshot;

                    CHECK(verifyScreenshot(vm, host, ss.str()));
                }
            }

            vm->CloseCart();
        }
    }
    */


    SUBCASE("Load simple cart"){
        vm->LoadCart("cartparsetest.p8", false);
        vm->vm_run();
        vm->Step(); //this will call the cart code, init, and update once

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("Frame count updated with each Step() Call"){
            vm->Step();
            vm->Step();

            CHECK(vm->GetFrameCount() == 3);
        }
        SUBCASE("check lua state"){
            //need to pass this func to the sandbox, not the global state
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
        vm->LoadCart("pset00-test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/pset00-test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pset 3 pix top left test cart renders correctly"){
        vm->LoadCart("pset3pix.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/pset3pix_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pset all pixels test cart renders correctly"){
        vm->LoadCart("psetall.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/psetall_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Clip test cart renders correctly"){
        vm->LoadCart("cliptest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/cliptest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Memory function test cart"){
        vm->LoadCart("memorytest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/memorytest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Cart data function test cart"){
        vm->LoadCart("cartdatatest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/cartdatatest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("tonum test cart"){
        vm->LoadCart("tonumtest2.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("can parse positive int"){
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r1test()\n"
                " return r1 == 12345\n"
                "end\n",
                "r1test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("can parse negative decimal") {
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r2test()\n"
                " return r2 == -12345.67\n"
                "end\n",
                "r2test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("can parse hex") {
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r3test()\n"
                " return r3 == 15\n"
                "end\n",
                "r3test");

            CHECK(parsedCorrectly);
        }
        
       SUBCASE("can parse binary literal") {
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r5test()\n"
                " return r5 == 9\n"
                "end\n",
                "r5test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("can parse large positive number") {
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r6test()\n"
                " return r6 == 32767\n"
                "end\n",
                "r6test");

            CHECK(parsedCorrectly);
        }
        SUBCASE("unparseable string returns nil") {
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r8test()\n"
                " return r8 == nil\n"
                "end\n",
                "r8test");

            CHECK(parsedCorrectly);
        }
        
        //currently failing
        SUBCASE("can parse hex with decimal") {
            vm->Step();

            bool parsedCorrectly = vm->ExecuteLua(
                "function r4test()\n"
                " return r4 == 15.6709\n"
                "end\n",
                "r4test");

            
            CHECK(parsedCorrectly);
        }
        //SUBCASE("too large int overflows") {
        //    vm->Step();

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
        vm->LoadCart("arithmetictest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("addition works"){
            vm->Step();

            bool additionWorks = vm->ExecuteLua(
                "function additiontest()\n"
                " return frames == 1\n"
                "end\n",
                "additiontest");

            CHECK(additionWorks);
        }
        SUBCASE("multiplication works"){
            vm->Step();

            bool multiplicationWorks = vm->ExecuteLua(
                "function multiplicationtest()\n"
                " return result == 10\n"
                "end\n",
                "multiplicationtest");

            CHECK(multiplicationWorks);
        }
        SUBCASE("division works"){
            vm->Step();

            bool divisionWorks = vm->ExecuteLua(
                "function divisiontest()\n"
                " return result2 == 0.2\n"
                "end\n",
                "divisiontest");

            CHECK(divisionWorks);
        }
        SUBCASE("subtraction works"){
            vm->Step();

            bool subtractionWorks = vm->ExecuteLua(
                "function subtractiontest()\n"
                " return result3 == 3.6\n"
                "end\n",
                "subtractiontest");

            CHECK(subtractionWorks);
        }
        SUBCASE("rnd returns between 0 and 1, and can add"){
            vm->Step();

            bool rndWorks = vm->ExecuteLua(
                "function rndtest()\n"
                " return result4 > 0 and result4 < 1\n"
                "end\n",
                "rndtest");

            CHECK(rndWorks);
        }
        SUBCASE("rnd returns between 0 and 1 without calling srand first"){
            vm->Step();

            bool rndWorks = vm->ExecuteLua(
                "function firstrndtest()\n"
                " return firstrand > 0 and firstrand < 1\n"
                "end\n",
                "firstrndtest");

            CHECK(rndWorks);
        }
        SUBCASE("# returns count of array"){
            vm->Step();

            bool rndWorks = vm->ExecuteLua(
                "function counttest()\n"
                " return tblcount == 4\n"
                "end\n",
                "counttest");

            CHECK(rndWorks);
        }
        SUBCASE("ceil works"){
            vm->Step();

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

        vm->LoadCart("cartparsetest.p8", false);
        vm->vm_run();
        vm->Step();

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
                "mapdraw", "extcmd", "next", "inext", "pairs", "ipairs",
                "rrect", "rrectfill"
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
        vm->LoadCart("reloadininit.p8", false);
        vm->vm_run();
        vm->Step();
        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
    }
    SUBCASE("rnd with table argument works") {
        vm->LoadCart("tablerndtest.p8", false);
        vm->vm_run();
        vm->Step();
        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
    }
    SUBCASE("Fill pattern test cart"){
        vm->LoadCart("fillptest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/fillptest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Peek4/poke4 test cart"){
        vm->LoadCart("peek4test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            //first frame doesn't have uniform colors, and since this is a 30 fps cart we have to step extra times
            vm->Step();
            vm->Step();
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/peek4test_f02.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pal (with table) test cart"){
        vm->LoadCart("paltabletest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/paltabletest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pairs() with nil arg test"){
        vm->LoadCart("nilpairstest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/nilpairstest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("split() test"){
        vm->LoadCart("splittest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/splittest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("General use memory persists across cart loads"){
        vm->LoadCart("cartparsetest.p8", false);
        vm->vm_run();
        vm->Step();
        auto origFirstByte = vm->vm_peek(0);
        vm->vm_poke(0x0000, 12);
        vm->vm_poke(0x42ff, 92);

        vm->vm_poke(0x4300, 33);
        vm->vm_poke(0x55ff, 129);

        vm->vm_poke(0x5600, 71);

        vm->vm_poke(0x5f00, 73);
        vm->vm_poke(0x7fff, 223);
        vm->LoadCart("cartparsetest.p8", false);

        CHECK_EQ(vm->vm_peek(0x0000), origFirstByte);
        CHECK_EQ(vm->vm_peek(0x42ff), 0);

        CHECK_EQ(vm->vm_peek(0x4300), 33);
        CHECK_EQ(vm->vm_peek(0x55ff), 129);

        CHECK_EQ(vm->vm_peek(0x5600), 71);
        CHECK_EQ(vm->vm_peek(0x5f00), 16);
        CHECK_EQ(vm->vm_peek(0x7fff), 0);


        vm->CloseCart();
    }
    SUBCASE("#include test"){
        vm->LoadCart("includetest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/includetest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("sub test"){
        vm->LoadCart("subtest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/subtest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("peek and poke extra args test"){
        vm->LoadCart("peek_poke_extraargs.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/peek_poke_extraargs_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("tline test"){
        vm->LoadCart("tline_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/tline_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("shorthand print (?) test"){
        vm->LoadCart("short_print_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/short_print_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("various pal args test"){
        vm->LoadCart("pal_args_test.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/pal_args_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("loop with max number value test"){
        vm->LoadCart("loop_max_val.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/loop_max_val_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("peek over 0x8000 works"){
        vm->LoadCart("peek_high_addr.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/peek_high_addr_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("ord with a high count doesn't crash"){
        vm->LoadCart("ord_multiple.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/ord_multiple_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("digits with no space before end and else can be parsed correctly"){
        vm->LoadCart("e_next_to_digit.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/e_next_to_digit_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("One off char printing"){
        vm->LoadCart("one_off_chars.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/one_off_chars_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("p8scii control code memory access"){
        vm->LoadCart("print_mem_poke.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/print_mem_poke_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("print scrolls screen"){
        vm->LoadCart("print_scroll_test.p8");
        vm->vm_run();
        vm->Step();
        
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/print_scroll_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("custom font test"){
        //font used from Pico World Race https://www.lexaloffle.com/bbs/?pid=106518
        //https://creativecommons.org/licenses/by-nc-sa/4.0/
        vm->LoadCart("ppwr-big-digit-test.p8");
        vm->vm_run();
        vm->Step();
        
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/ppwr-big-digit-test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("split with no args test"){
        vm->LoadCart("split_noargs_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/split_noargs_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("string bracket indexing and sub test"){
        vm->LoadCart("str_index_sub_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/str_index_sub_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("count with val arg test"){
        vm->LoadCart("count_val_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/count_val_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("bold text with wide char test"){
        vm->LoadCart("boldtexttest.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/boldtexttest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("peek with large count test"){
        vm->LoadCart("peek_large_count.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/peek_large_count_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("chr with large args test"){
        vm->LoadCart("chr_large_args.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/chr_large_args_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("ord with nil arg test"){
        vm->LoadCart("ord_nil_arg.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/ord_nil_arg_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("p8scii solid bg with custom font test"){
        vm->LoadCart("p8scii_bg_custom_font_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/p8scii_bg_custom_font_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Per character width adjustment test"){
        vm->LoadCart("per_char_width_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/per_char_width_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("color with negative number test"){
        vm->LoadCart("neg_scrn_pal_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/neg_scrn_pal_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("env modification test"){
        vm->LoadCart("nested_env_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/nested_env_test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("tilde (~) operator (bxor) test"){
        vm->LoadCart("tilde_bxor_test.p8", false);
        vm->vm_run();
        vm->Step();

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("screen matches screenshot"){
            vm->Step();

            CHECK(verifyScreenshot(vm, host, "carts/screenshots/tilde_bxor_test_f01.png"));
        }

        vm->CloseCart();
    }
    
    delete vm;
    delete host;
}
#include <string>
#include <vector>

#include "doctest.h"
#include "../libs/lodepng/lodepng.h"

#include "../source/vm.h"
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

        bool pixelMatches = r == col.Red && g == col.Green && b == col.Blue;

        if (!pixelMatches) {
            
            //printf("Non-matching pixel at idx %d (%d,%d): rgb: %d,%d,%d\n", pixIdx, x, y, r, g, b);

            //printf("fake 08 color is %d rgb: %d,%d,%d\n", c, col.Red, col.Green, col.Blue);
        }

        pixelsMatch &= pixelMatches;
    }

    return pixelsMatch;

}

TEST_CASE("Loading and running carts") {
    Vm* vm = new Vm();

    SUBCASE("Load simple cart"){
        vm->LoadCart("carts/cartparsetest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("Frame count updated with each UpdateAndDrawCall()")
        {
            vm->UpdateAndDraw(0, 0);
            vm->UpdateAndDraw(0, 0);
            vm->UpdateAndDraw(0, 0);

            CHECK(vm->GetFrameCount() == 3);
        }
        SUBCASE("check lua state")
        {
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
        vm->LoadCart("carts/pset00-test.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot")
        {
            vm->UpdateAndDraw(0, 0);

            CHECK(verifyScreenshot(vm, "carts/screenshots/pset00-test_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pset 3 pix top left test cart renders correctly"){
        vm->LoadCart("carts/pset3pix.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot")
        {
            vm->UpdateAndDraw(0, 0);

            CHECK(verifyScreenshot(vm, "carts/screenshots/pset3pix_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("pset all pixels test cart renders correctly"){
        vm->LoadCart("carts/psetall.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot")
        {
            vm->UpdateAndDraw(0, 0);

            CHECK(verifyScreenshot(vm, "carts/screenshots/psetall_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Clip test cart renders correctly"){
        vm->LoadCart("carts/cliptest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot")
        {
            vm->UpdateAndDraw(0, 0);

            CHECK(verifyScreenshot(vm, "carts/screenshots/cliptest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Memory function test cart"){
        vm->LoadCart("carts/memorytest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot")
        {
            vm->UpdateAndDraw(0, 0);

            CHECK(verifyScreenshot(vm, "carts/screenshots/memorytest_f01.png"));
        }

        vm->CloseCart();
    }
    SUBCASE("Cart data function test cart"){
        vm->LoadCart("carts/cartdatatest.p8");

        SUBCASE("No error reported"){
            CHECK(vm->GetBiosError() == "");
        }
        SUBCASE("sceen matches screenshot")
        {
            vm->UpdateAndDraw(0, 0);

            CHECK(verifyScreenshot(vm, "carts/screenshots/cartdatatest_f01.png"));
        }

        vm->CloseCart();
    }

    delete vm;
}
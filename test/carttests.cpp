#include "doctest.h"
#include "../source/cart.h"

TEST_CASE("Loads bios cart") {
    Cart* cart = new Cart("__FAKE08-BIOS.p8");

    SUBCASE("filename is correct") {
        CHECK(cart->Filename == "__FAKE08-BIOS.p8");
    }
    SUBCASE("error is empty") {
        CHECK(cart->LoadError == "");
    }
    SUBCASE("Lua section is populated") {
        CHECK(cart->LuaString.length() > 100);
    }
    SUBCASE("Gfx section is populated") {
        CHECK(cart->SpriteSheetString.length() > 100);
    }
    SUBCASE("Sprite flags section is empty") {
        CHECK(cart->SpriteFlagsString.length() == 0);
    }
    SUBCASE("Map section is empty") {
        CHECK(cart->MapString.length() == 0);
    }
    SUBCASE("Sfx section is empty") {
        CHECK(cart->SfxString.length() == 0);
    }
    SUBCASE("Music section is empty") {
        CHECK(cart->MusicString.length() == 0);
    }
    
    delete cart;
}

TEST_CASE("Load simple p8 cart") {
    Cart* cart = new Cart("carts/cartparsetest.p8");

    SUBCASE("filename is correct") {
        CHECK(cart->Filename == "carts/cartparsetest.p8");
    }
    SUBCASE("error is empty") {
        CHECK(cart->LoadError == "");
    }
    SUBCASE("Lua section is populated") {
        CHECK(cart->LuaString == "a=1\n");
    }
    SUBCASE("Gfx section is populated") {
        CHECK(cart->SpriteSheetString ==
            "ff102030555555550000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    }
    SUBCASE("Sprite flags section is populated") {
        CHECK(cart->SpriteFlagsString == 
            "0001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    }
    SUBCASE("Map section is populated") {
        CHECK(cart->MapString ==
            "0100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n");
    }
    SUBCASE("Sfx section is populated") {
        CHECK(cart->SfxString == 
            "001d00000d0300e0500e0500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n0010000036050310502a05019050192501925019250192501925019250192501925019250192501b2501b250192501b2501b2501b2501b2501b2501b2501b2501b2501925019250192501b250000000000000000\n");
    }
    SUBCASE("Music section is populated") {
        CHECK(cart->MusicString == "00 01424344\n\n");
    }
    SUBCASE("Gfx data is populated") {
        CHECK(cart->SpriteSheetData[0] == 255);
        CHECK(cart->SpriteSheetData[1] == 1);
        CHECK(cart->SpriteSheetData[2] == 2);
        CHECK(cart->SpriteSheetData[3] == 3);
    }
    SUBCASE("Sprite flags is populated") {
        CHECK(cart->SpriteFlagsData[0] == 0);
        CHECK(cart->SpriteFlagsData[1] == 1);
    }
    SUBCASE("Map data is populated") {
        CHECK(cart->MapData[0] == 1);
        CHECK(cart->MapData[1] == 0);
    }
    SUBCASE("Sfx data is populated") {
        CHECK(cart->SfxData[0].data[0] == 13);
        CHECK(cart->SfxData[0].editorMode == 0);
        CHECK(cart->SfxData[0].loopRangeEnd == 0);
        CHECK(cart->SfxData[0].loopRangeEnd == 0);
        CHECK(cart->SfxData[0].speed == 29);

        CHECK(cart->SfxData[0].notes[0].waveform == 0);
        CHECK(cart->SfxData[0].notes[0].effect == 0);
        CHECK(cart->SfxData[0].notes[0].key == 13);
        CHECK(cart->SfxData[0].notes[0].volume == 3);
        CHECK(cart->SfxData[0].notes[0].data[0] == 13);
        CHECK(cart->SfxData[0].notes[0].data[1] == 6);
    }
    SUBCASE("Music data is populated") {
        CHECK(cart->SongData[0].sfx0 == 1);
        CHECK(cart->SongData[0].start == 0);
        CHECK(cart->SongData[0].sfx1 == 66);
        CHECK(cart->SongData[0].loop == 0);
        CHECK(cart->SongData[0].sfx2 == 67);
        CHECK(cart->SongData[0].stop == 0);
        CHECK(cart->SongData[0].sfx3 == 68);
        CHECK(cart->SongData[0].mode == 0);

    }

    delete cart;
}

TEST_CASE("Load simple png cart") {
    Cart* cart = new Cart("carts/cartparsetest.p8.png");

    SUBCASE("filename is correct") {
        CHECK(cart->Filename == "carts/cartparsetest.p8.png");
    }
    SUBCASE("error is empty") {
        CHECK(cart->LoadError == "");
    }
    SUBCASE("Lua section is populated") {
        CHECK(cart->LuaString == "a=1");
    }
    SUBCASE("Gfx data is populated") {
        CHECK(cart->SpriteSheetData[0] == 255);
        CHECK(cart->SpriteSheetData[1] == 1);
        CHECK(cart->SpriteSheetData[2] == 2);
        CHECK(cart->SpriteSheetData[3] == 3);
    }
    SUBCASE("Sprite flags is populated") {
        CHECK(cart->SpriteFlagsData[0] == 0);
        CHECK(cart->SpriteFlagsData[1] == 1);
    }
    SUBCASE("Map data is populated") {
        CHECK(cart->MapData[0] == 1);
        CHECK(cart->MapData[1] == 0);
    }
    SUBCASE("Sfx data is populated") {
        CHECK(cart->SfxData[0].data[0] == 13);
        CHECK(cart->SfxData[0].editorMode == 0);
        CHECK(cart->SfxData[0].loopRangeEnd == 0);
        CHECK(cart->SfxData[0].loopRangeEnd == 0);
        CHECK(cart->SfxData[0].speed == 29);

        CHECK(cart->SfxData[0].notes[0].waveform == 0);
        CHECK(cart->SfxData[0].notes[0].effect == 0);
        CHECK(cart->SfxData[0].notes[0].key == 13);
        CHECK(cart->SfxData[0].notes[0].volume == 3);
        CHECK(cart->SfxData[0].notes[0].data[0] == 13);
        CHECK(cart->SfxData[0].notes[0].data[1] == 6);
    }
    SUBCASE("Music data is populated") {
        CHECK(cart->SongData[0].sfx0 == 1);
        CHECK(cart->SongData[0].start == 0);
        CHECK(cart->SongData[0].sfx1 == 66);
        CHECK(cart->SongData[0].loop == 0);
        CHECK(cart->SongData[0].sfx2 == 67);
        CHECK(cart->SongData[0].stop == 0);
        CHECK(cart->SongData[0].sfx3 == 68);
        CHECK(cart->SongData[0].mode == 0);

    }

    delete cart;
}

TEST_CASE("Load legacy png cart") {
    //this cart downloaded from picotool's tests
    //https://github.com/dansanderson/picotool/blob/master/tests/testdata/test_cart.p8.png
    //MIT license
    //Might be good to have a more complex test cart for this scenario, but for now
    //just making sure its decompressed is better than nothing
    Cart* cart = new Cart("carts/test_legacypng_cart.p8.png");

    SUBCASE("filename is correct") {
        CHECK(cart->Filename == "carts/test_legacypng_cart.p8.png");
    }
    SUBCASE("error is empty") {
        CHECK(cart->LoadError == "");
    }
    SUBCASE("Lua section is populated") {
        CHECK(cart->LuaString == "print(\"0.1.10c\")\n");
    }
    SUBCASE("Gfx data is populated") {
        CHECK(cart->SpriteSheetData[0] == 0);
        CHECK(cart->SpriteSheetData[1] == 0);
        CHECK(cart->SpriteSheetData[2] == 0);
        CHECK(cart->SpriteSheetData[3] == 0);
    }

    delete cart;
}

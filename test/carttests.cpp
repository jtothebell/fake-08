#include "doctest.h"
#include "../source/cart.h"

TEST_CASE("Loads bios cart") {
    Cart* cart = new Cart("__FAKE08-BIOS.p8", "");

    SUBCASE("FullCartPath is correct") {
        CHECK(cart->FullCartPath == "__FAKE08-BIOS.p8");
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
    Cart* cart = new Cart("carts/cartparsetest.p8", "");

    SUBCASE("FullCartPath is correct") {
        CHECK(cart->FullCartPath == "carts/cartparsetest.p8");
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
            "001d00000d0300e0500e0500000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\n"
            "0010000036050310502a05019050192501925019250192501925019250192501925019250192501b2501b250192501b2501b2501b2501b2501b2501b2501b2501b2501925019250192501b250000000000000000\n"
            "046b82a00b223183741851416646098310bb5525a662a53038f253c23527d2011a240a3653fe7108e42307353ce023fa161b9423a7420de0234507314070543703766344043dc0433a3419e63358020cb3308266\n");
    }
    SUBCASE("Music section is populated") {
        CHECK(cart->MusicString == "00 01424344\n0d 1b7d6f45\n\n");
    }
    SUBCASE("Gfx data is populated") {
        CHECK(cart->CartRom.SpriteSheetData[0] == 255);
        CHECK(cart->CartRom.SpriteSheetData[1] == 1);
        CHECK(cart->CartRom.SpriteSheetData[2] == 2);
        CHECK(cart->CartRom.SpriteSheetData[3] == 3);
    }
    SUBCASE("Sprite flags is populated") {
        CHECK(cart->CartRom.SpriteFlagsData[0] == 0);
        CHECK(cart->CartRom.SpriteFlagsData[1] == 1);
    }
    SUBCASE("Map data is populated") {
        CHECK(cart->CartRom.MapData[0] == 1);
        CHECK(cart->CartRom.MapData[1] == 0);
    }
    SUBCASE("Sfx data is populated") {
        CHECK(cart->CartRom.SfxData[0].data[0] == 13);
        CHECK(cart->CartRom.SfxData[0].editorMode == 0);
        CHECK(cart->CartRom.SfxData[0].loopRangeEnd == 0);
        CHECK(cart->CartRom.SfxData[0].loopRangeEnd == 0);
        CHECK(cart->CartRom.SfxData[0].speed == 29);

        CHECK(cart->CartRom.SfxData[0].notes[0].data[0] == 13);
        CHECK(cart->CartRom.SfxData[0].notes[0].data[1] == 6);

        CHECK(cart->CartRom.SfxData[0].notes[0].getWaveform() == 0);
        CHECK(cart->CartRom.SfxData[0].notes[0].getEffect() == 0);
        CHECK(cart->CartRom.SfxData[0].notes[0].getKey() == 13);
        CHECK(cart->CartRom.SfxData[0].notes[0].getVolume() == 3);
        CHECK(cart->CartRom.SfxData[0].notes[0].getCustom() == 0);
    }
    SUBCASE("Sfx data byte array is populated correctly") {
        CHECK(cart->CartRom.SfxData[2].data[0] == 139);
        CHECK(cart->CartRom.SfxData[2].data[1] == 52);
        CHECK(cart->CartRom.SfxData[2].data[2] == 216);
        CHECK(cart->CartRom.SfxData[2].data[3] == 78);
        CHECK(cart->CartRom.SfxData[2].data[4] == 88);
        CHECK(cart->CartRom.SfxData[2].data[5] == 67);
        CHECK(cart->CartRom.SfxData[2].data[6] == 150);
        CHECK(cart->CartRom.SfxData[2].data[7] == 105);
        CHECK(cart->CartRom.SfxData[2].data[8] == 9);
        CHECK(cart->CartRom.SfxData[2].data[9] == 150);
        CHECK(cart->CartRom.SfxData[2].data[10] == 203);
        CHECK(cart->CartRom.SfxData[2].data[11] == 218);
        CHECK(cart->CartRom.SfxData[2].data[12] == 165);
        CHECK(cart->CartRom.SfxData[2].data[13] == 236);
    }
    SUBCASE("Sfx not set by string defaults correctly") {
        //speed doesn't default to 0
        CHECK(cart->CartRom.SfxData[63].speed == 16);
        CHECK(cart->CartRom.SfxData[63].data[65] == 16);
    }
    SUBCASE("Music song data is populated") {
        CHECK(cart->CartRom.SongData[0].getSfx0() == 1);
        CHECK(cart->CartRom.SongData[0].getStart() == 0);
        CHECK(cart->CartRom.SongData[0].getSfx1() == 66);
        CHECK(cart->CartRom.SongData[0].getLoop() == 0);
        CHECK(cart->CartRom.SongData[0].getSfx2() == 67);
        CHECK(cart->CartRom.SongData[0].getStop() == 0);
        CHECK(cart->CartRom.SongData[0].getSfx3() == 68);
        CHECK(cart->CartRom.SongData[0].getMode() == 0);
    }
    SUBCASE("Music data byte array is populated correctly") {
        CHECK(cart->CartRom.SongData[1].data[0] == 155);
        CHECK(cart->CartRom.SongData[1].data[1] == 125);
        CHECK(cart->CartRom.SongData[1].data[2] == 239);
        CHECK(cart->CartRom.SongData[1].data[3] == 197);
    }

    delete cart;
}

TEST_CASE("Load simple png cart") {
    Cart* cart = new Cart("cartparsetest.p8.png", "carts");

    SUBCASE("FullCartPath is correct") {
        CHECK(cart->FullCartPath == "carts/cartparsetest.p8.png");
    }
    SUBCASE("error is empty") {
        CHECK(cart->LoadError == "");
    }
    SUBCASE("Lua section is populated") {
        CHECK(cart->LuaString == "a=1");
    }
    SUBCASE("Gfx data is populated") {
        CHECK(cart->CartRom.SpriteSheetData[0] == 255);
        CHECK(cart->CartRom.SpriteSheetData[1] == 1);
        CHECK(cart->CartRom.SpriteSheetData[2] == 2);
        CHECK(cart->CartRom.SpriteSheetData[3] == 3);
    }
    SUBCASE("Sprite flags is populated") {
        CHECK(cart->CartRom.SpriteFlagsData[0] == 0);
        CHECK(cart->CartRom.SpriteFlagsData[1] == 1);
    }
    SUBCASE("Map data is populated") {
        CHECK(cart->CartRom.MapData[0] == 1);
        CHECK(cart->CartRom.MapData[1] == 0);
    }
    SUBCASE("Sfx data is populated") {
        CHECK(cart->CartRom.SfxData[0].data[0] == 13);
        CHECK(cart->CartRom.SfxData[0].editorMode == 0);
        CHECK(cart->CartRom.SfxData[0].loopRangeEnd == 0);
        CHECK(cart->CartRom.SfxData[0].loopRangeEnd == 0);
        CHECK(cart->CartRom.SfxData[0].speed == 29);

        CHECK(cart->CartRom.SfxData[0].notes[0].getWaveform() == 0);
        CHECK(cart->CartRom.SfxData[0].notes[0].getEffect() == 0);
        CHECK(cart->CartRom.SfxData[0].notes[0].getKey() == 13);
        CHECK(cart->CartRom.SfxData[0].notes[0].getVolume() == 3);
        CHECK(cart->CartRom.SfxData[0].notes[0].data[0] == 13);
        CHECK(cart->CartRom.SfxData[0].notes[0].data[1] == 6);
    }
    SUBCASE("Music data is populated") {
        CHECK(cart->CartRom.SongData[0].getSfx0() == 1);
        CHECK(cart->CartRom.SongData[0].getStart() == 0);
        CHECK(cart->CartRom.SongData[0].getSfx1() == 66);
        CHECK(cart->CartRom.SongData[0].getLoop() == 0);
        CHECK(cart->CartRom.SongData[0].getSfx2() == 67);
        CHECK(cart->CartRom.SongData[0].getStop() == 0);
        CHECK(cart->CartRom.SongData[0].getSfx3() == 68);
        CHECK(cart->CartRom.SongData[0].getMode() == 0);
    }

    delete cart;
}

TEST_CASE("Load legacy png cart") {
    //this cart downloaded from picotool's tests
    //https://github.com/dansanderson/picotool/blob/master/tests/testdata/test_cart.p8.png
    //MIT license
    //Might be good to have a more complex test cart for this scenario, but for now
    //just making sure its decompressed is better than nothing
    Cart* cart = new Cart("test_legacypng_cart.p8.png", "carts");

    SUBCASE("FullCartPath is correct") {
        CHECK(cart->FullCartPath == "carts/test_legacypng_cart.p8.png");
    }
    SUBCASE("error is empty") {
        CHECK(cart->LoadError == "");
    }
    SUBCASE("Lua section is populated") {
        CHECK(cart->LuaString == "print(\"0.1.10c\")\n");
    }
    SUBCASE("Gfx data is populated") {
        CHECK(cart->CartRom.SpriteSheetData[0] == 0);
        CHECK(cart->CartRom.SpriteSheetData[1] == 0);
        CHECK(cart->CartRom.SpriteSheetData[2] == 0);
        CHECK(cart->CartRom.SpriteSheetData[3] == 0);
    }

    delete cart;
}

TEST_CASE("cart loading options") {
    Cart* cart = new Cart("#cartparsetest", "carts");

    SUBCASE("FullCartPath is correct") {
        CHECK(cart->FullCartPath == "carts/cartparsetest.p8");
    }

    delete cart;
}
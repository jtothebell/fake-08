#include <string>
#include <vector>

#include "doctest.h"

#include "../source/vm.h"
#include "../source/hostVmShared.h"
#include "../source/host.h"
#include "stubhost.h"

#include "../source/fontdata.h"


TEST_CASE("Vm memory functions") {
    StubHost* stubHost = new StubHost();
    PicoRam* memory = new PicoRam();
    memory->Reset();
    Graphics* graphics = new Graphics(get_font_data(), memory);
    Input* input = new Input(memory);
    Audio* audio = new Audio(memory);

    Vm* vm = new Vm(stubHost, memory, graphics, input, audio);

    SUBCASE("memory data stats with 0"){
        CHECK(memory->data[0] == 0);
    }
    SUBCASE("resetting memory zeroes out everything except general use, color bitmask, screen data mapping, and map width"){
        for(int i = 0; i < 0x10000; ++i) {
            memory->data[i] = i & 255;
        }

        memory->Reset();

        bool correctValues = true;
        for(int i = 0; i < 0x8000; ++i) {
            if (i == 0x5f5e) {
                correctValues &= memory->data[i] == 255;
            }
            else if (i == 0x5f55) {
                correctValues &= memory->data[i] == 0x60;
            }
            else if (i == 0x5f56) {
                correctValues &= memory->data[i] == 0x20;
            }
            else if (i == 0x5f57) {
                correctValues &= memory->data[i] == 128;
            }
            else if (i < 0x4300 || i > 0x5eff) {
                correctValues &= memory->data[i] == 0;
            }
        }

        CHECK(correctValues);
    }
    SUBCASE("size of PicoRam struct corrct"){
        CHECK(sizeof(PicoRam) == 0x10000);
    }
    SUBCASE("size of data correct"){
        CHECK(sizeof(memory->data) == 0x10000);
    }
    SUBCASE("simple peek and poke"){
        vm->vm_poke(2, 232);

        CHECK(vm->vm_peek(2) == 232);
    }
    SUBCASE("simple peek2 and poke2"){
        vm->vm_poke2(41, -1031);

        CHECK(vm->vm_peek2(41) == -1031);
    }
    SUBCASE("simple peek4 and poke4"){
        vm->vm_poke4(0x7123, 3584.0059);

        CHECK(vm->vm_peek4(0x7123).bits() == ((z8::fix32)3584.0059).bits());
    }
    SUBCASE("poking spritesheet"){
        //147: 1001 0011
        //left pixel: 0011 = 3
        //right pixel: 1001 = 9
        vm->vm_poke(0, 147);

        CHECK_EQ(graphics->sget(0, 0), 3);
        CHECK_EQ(graphics->sget(1, 0), 9);
    }
    SUBCASE("poking map"){
        vm->vm_poke(0x2000, 201);
        vm->vm_poke(0x2003, 11);

        CHECK_EQ(graphics->mget(0, 0), 201);
        CHECK_EQ(graphics->mget(3, 0), 11);
    }
    SUBCASE("poking sprite flags"){
        //39: 0010 0111
        vm->vm_poke(0x3000, 39);

        CHECK_EQ(graphics->fget(0), 39);
        CHECK_EQ(graphics->fget(0, 0), true);
        CHECK_EQ(graphics->fget(0, 3), false);
    }
    SUBCASE("poking music"){
        //78: 0100 1110
        vm->vm_poke(0x3100, 78);

        CHECK_EQ(memory->songs[0].getSfx0(), 78);
        CHECK_EQ(memory->songs[0].getStart(), 0);
    }
    SUBCASE("poking sfx"){
        //97: 0110 0001
        //key is first 6 bits: 10 0001 : 33
        //waveform is next 3: (0) 01 : 1
        vm->vm_poke(0x3200, 97);

        CHECK_EQ(memory->sfx[0].notes[0].getKey(), 33);
        CHECK_EQ(memory->sfx[0].notes[0].getWaveform(), 1);
    }
    SUBCASE("poking general use ram"){
        vm->vm_poke(0x4300, 215);

        CHECK_EQ(memory->generalUseRam[0], 215);
    }
    SUBCASE("setting cart data"){
        vm->vm_cartdata("dummy");
        vm->vm_dset(13, 56);

        CHECK_EQ(vm->vm_dget(13), (fix32)56);
    }
    SUBCASE("poking cart data"){
        //need to poke after cartdata is called to set the current key, otherwise it will get wiped
        vm->vm_cartdata("dummy");
        vm->vm_poke(0x5e02, 56);

        CHECK_EQ(memory->cartData[2], 56);
        CHECK_EQ(vm->vm_dget(0), (fix32)56);
    }
    SUBCASE("poking and peeking cart data"){
        vm->vm_cartdata("dummy");
        vm->vm_poke4(0x5e80, (fix32)133);

        CHECK_EQ(vm->vm_peek4(0x5e80), (fix32)133);
        CHECK_EQ(vm->vm_dget(32), (fix32)133);
    }
    SUBCASE("poking draw palette data"){
        //21 : 0001 0101 (transparet: true) (color mapped 5)
        vm->vm_poke(0x5f02, 21);

        CHECK_EQ(memory->drawState.drawPaletteMap[2], 21);
        CHECK_EQ(graphics->getDrawPalMappedColor(2), 5);
        CHECK_EQ(graphics->isColorTransparent(2), true);
    }
    SUBCASE("poking clip data"){
        //7 : 0000 0111
        vm->vm_poke(0x5f21, 89);

        CHECK_EQ(memory->drawState.clip_yb, 89);
    }
    SUBCASE("poking unknown ram at 0x5f24"){
        //7 : 0000 0111
        vm->vm_poke(0x5f24, 254);

        CHECK_EQ(memory->drawState.unknown05f24, 254);
    }
    SUBCASE("poking color"){
        vm->vm_poke(0x5f25, 12);

        CHECK_EQ(memory->drawState.color, 12);
    }
    SUBCASE("poking text cursor"){
        vm->vm_poke(0x5f26, 74);

        CHECK_EQ(memory->drawState.text_x, 74);
    }
    SUBCASE("poking camera position"){
        vm->vm_poke2(0x5f2a, -72);

        CHECK_EQ(memory->drawState.camera_y, -72);
    }
    SUBCASE("poking draw mode"){
        vm->vm_poke(0x5f2c, 2);

        CHECK_EQ(memory->drawState.drawMode, 2);
    }
    SUBCASE("poking dev kit mode"){
        vm->vm_poke(0x5f2d, 2);

        CHECK_EQ(memory->drawState.devkitMode, 2);
    }
    SUBCASE("poking persist palette"){
        vm->vm_poke(0x5f2e, 1);

        CHECK_EQ(memory->drawState.persistPalette, 1);
    }
    SUBCASE("poking sound pause state"){
        vm->vm_poke(0x5f2f, 4);

        CHECK_EQ(memory->drawState.soundPauseState, 4);
    }
    SUBCASE("poking suppress pause"){
        vm->vm_poke(0x5f30, 1);

        CHECK_EQ(memory->drawState.suppressPause, 1);
    }
    SUBCASE("poking fill pattern"){
        vm->vm_poke(0x5f31, 24);

        CHECK_EQ(memory->drawState.fillPattern[0], 24);
    }
    SUBCASE("poking fill pattern transparency bit"){
        vm->vm_poke(0x5f33, 1);

        CHECK_EQ(memory->drawState.fillPatternTransparencyBit, 1);
    }
    SUBCASE("poking color setting flag"){
        /*
-- bit  0x1000.0000 means the non-colour bits should be observed
-- bit  0x0100.0000 transparency bit
-- bits 0x00FF.0000 are the usual colour bits
-- bits 0x0000.FFFF are interpreted as the fill pattern
        */
        vm->vm_poke(0x5f34, 36);

        CHECK_EQ(memory->drawState.colorSettingFlag, 36);
    }
    SUBCASE("poking line invalid"){
        vm->vm_poke(0x5f35, 1);

        CHECK_EQ(memory->drawState.lineInvalid, 1);
    }
    SUBCASE("poking tline width"){
        vm->vm_poke(0x5f38, 65);

        CHECK_EQ(memory->drawState.tlineMapWidth, 65);
    }
    SUBCASE("poking tline height"){
        vm->vm_poke(0x5f39, 66);

        CHECK_EQ(memory->drawState.tlineMapHeight, 66);
    }
    SUBCASE("poking tline x"){
        vm->vm_poke(0x5f3a, 67);

        CHECK_EQ(memory->drawState.tlineMapXOffset, 67);
    }
    SUBCASE("poking tline y"){
        vm->vm_poke(0x5f3b, 68);

        CHECK_EQ(memory->drawState.tlineMapYOffset, 68);
    }
    SUBCASE("poking line x"){
        vm->vm_poke2(0x5f3c, -3);

        CHECK_EQ(memory->drawState.line_x, -3);
    }
    SUBCASE("poking line y"){
        vm->vm_poke2(0x5f3e, 262);

        CHECK_EQ(memory->drawState.line_y, 262);
    }
    SUBCASE("poking audio hardware state half rate") {
        vm->vm_poke(0x5f40, 3);

        CHECK_EQ(memory->hwState.half_rate, 3);
    }
    SUBCASE("poking audio hardware state distort") {
        vm->vm_poke(0x5f42, 5);

        CHECK_EQ(memory->hwState.distort, 5);
    }
    SUBCASE("poking rng state") {
        //rng gets seeded, we need to zero it out to test this easier
        memory->hwState.rngState[0] = 0;
        vm->vm_poke(0x5f44, 20);

        CHECK_EQ(memory->hwState.rngState[0], 20);
    }
    SUBCASE("poking button state doesn't affect input") {
        //0000 1010
        vm->vm_poke(0x5f4c, 10);

        CHECK_EQ(memory->hwState.buttonStates[0], 10);
        CHECK_EQ(input->btn(), 0);
    }
    SUBCASE("poking print attributes") {
        vm->vm_poke(0x5f58, 53);

        CHECK_EQ(memory->hwState.printAttributes, 53);
    }
    SUBCASE("poking btnp repeat delay") {
        vm->vm_poke(0x5f5c, 10);

        CHECK_EQ(memory->hwState.btnpRepeatDelay, 10);
    }
    SUBCASE("poking btnp repeat interval") {
        vm->vm_poke(0x5f5d, 25);

        CHECK_EQ(memory->hwState.btnpRepeatInterval, 25);
    }
    SUBCASE("poking color bitmask") {
        vm->vm_poke(0x5f5e, 42);

        CHECK_EQ(memory->hwState.colorBitmask, 42);
    }
    SUBCASE("poking alternate palette flag") {
        vm->vm_poke(0x5f5f, 10);

        CHECK_EQ(memory->hwState.alternatePaletteFlag, 10);
    }
    SUBCASE("poking alternate palette map") {
        vm->vm_poke(0x5f60, 11);

        CHECK_EQ(memory->hwState.alternatePaletteMap[0], 11);
    }
    SUBCASE("poking alternate palette map") {
        vm->vm_poke(0x5f70, 7);

        CHECK_EQ(memory->hwState.alternatePaletteScreenLineBitfield[0], 7);
    }
    SUBCASE("poking gpio pins") {
        vm->vm_poke(0x5f80, 192);

        CHECK_EQ(memory->hwState.gpioPins[0], 192);
    }
    SUBCASE("poking screen data (first two pixels)") {
        //195: 1100 0011 (left pixel 3, right pixel 12)
        vm->vm_poke(0x6000, 195);

        CHECK_EQ(memory->screenBuffer[0], 195);
        CHECK_EQ(graphics->pget(0, 0), 3);
        CHECK_EQ(graphics->pget(1, 0), 12);
    }
    SUBCASE("poking screen data (last two pixels)") {
        //210: 1101 0010
        vm->vm_poke(0x7fff, 210);

        CHECK_EQ(memory->screenBuffer[8191], 210);
        CHECK_EQ(graphics->pget(126, 127), 2);
        CHECK_EQ(graphics->pget(127, 127), 13);
    }
    SUBCASE("poking user data (first byte)") {
        vm->vm_poke(0x8000, 129);

        CHECK_EQ(memory->data[0x8000], 129);
        CHECK_EQ(memory->userData[0], 129);
    }
    SUBCASE("poking user data (last byte)") {
        vm->vm_poke(0xffff, 211);

        CHECK_EQ(memory->data[0xffff], 211);
        CHECK_EQ(memory->userData[32767], 211);
    }
    SUBCASE("Loading cart copies cart rom to memory") {
        vm->LoadCart("cartparsetest.p8.png");

        SUBCASE("Gfx data is populated") {
            CHECK(memory->spriteSheetData[0] == 255);
            CHECK(memory->spriteSheetData[1] == 1);
            CHECK(memory->spriteSheetData[2] == 2);
            CHECK(memory->spriteSheetData[3] == 3);
        }
        SUBCASE("Sprite flags is populated") {
            CHECK(memory->spriteFlags[0] == 0);
            CHECK(memory->spriteFlags[1] == 1);
        }
        SUBCASE("Map data is populated") {
            CHECK(memory->mapData[0] == 1);
            CHECK(memory->mapData[1] == 0);
        }
        SUBCASE("Sfx data is populated") {
            CHECK(memory->sfx[0].data[0] == 13);
            CHECK(memory->sfx[0].filters == 0);
            CHECK(memory->sfx[0].loopRangeEnd == 0);
            CHECK(memory->sfx[0].loopRangeEnd == 0);
            CHECK(memory->sfx[0].speed == 29);

            CHECK(memory->sfx[0].notes[0].getWaveform() == 0);
            CHECK(memory->sfx[0].notes[0].getEffect() == 0);
            CHECK(memory->sfx[0].notes[0].getKey() == 13);
            CHECK(memory->sfx[0].notes[0].getVolume() == 3);
            CHECK(memory->sfx[0].notes[0].data[0] == 13);
            CHECK(memory->sfx[0].notes[0].data[1] == 6);
        }
        SUBCASE("Music data is populated") {
            CHECK(memory->songs[0].getSfx0() == 1);
            CHECK(memory->songs[0].getStart() == 0);
            CHECK(memory->songs[0].getSfx1() == 66);
            CHECK(memory->songs[0].getLoop() == 0);
            CHECK(memory->songs[0].getSfx2() == 67);
            CHECK(memory->songs[0].getStop() == 0);
            CHECK(memory->songs[0].getSfx3() == 68);
            CHECK(memory->songs[0].getMode() == 0);
        }
    }
    SUBCASE("reload writes cart rom over changes") {
        vm->LoadCart("cartparsetest.p8.png");

        graphics->sset(0, 0, 4);
        graphics->fset(0, 3);
        graphics->mset(0, 0, 10);

        vm->vm_reload(0, 0, 0x4300, "");

        CHECK_EQ(graphics->sget(0, 0), 15);
        CHECK_EQ(graphics->fget(0), 0);
        CHECK_EQ(graphics->mget(0, 0), 1);
    }
    SUBCASE("memset sets memory") {
        vm->vm_memset(0x3000, 13, 4);

        CHECK_EQ(graphics->fget(0), 13);
        CHECK_EQ(graphics->fget(3), 13);
    }
    SUBCASE("memcpy copies memory") {
        vm->LoadCart("cartparsetest.p8.png");
        vm->vm_memcpy(0x3000, 0, 4);

        CHECK_EQ(memory->spriteSheetData[1], 1);
    }
    SUBCASE("dget before setting cart data is allowed for backward compat") {
        vm->vm_dset(34, 1925);

        CHECK_EQ(vm->vm_dget(34), (fix32)1925);
    }
    SUBCASE("dget after setting cart data key stores value") {
        vm->vm_cartdata("uniquekey");
        vm->vm_dset(34, 1923);

        CHECK_EQ(vm->vm_dget(34), (fix32)1923);
    }
    SUBCASE("pico driller style input"){
        vm->LoadCart("drillerinputtest.p8");
        //this loads the cart + glue code into a coroutine, but does not execute anything yet
        //should probably get renamed. Same as typing LOAD() in pico 8
        vm->vm_run(); 
        vm->Step(); //this will call the cart code, init, and update once
        //30 fps game loop requires 1 step to update and update buttons, and another to draw

        SUBCASE("initial state"){
            bool btnbits = vm->ExecuteLua(
                "function btnbitsinitialtest()\n"
                " return btnbits == 0\n"
                "end\n",
                "btnbitsinitialtest");

            CHECK(btnbits);
        }

        SUBCASE("no buttons gives 0"){
            vm->Step();
            bool btnbits = vm->ExecuteLua(
                "function btnbitstest0()\n"
                " return btnbits == 0\n"
                "end\n",
                "btnbitstest0");

            CHECK(btnbits);
        }
        SUBCASE("left pushed returns 1"){
            stubHost->stubInput(1, 1);
            //requires two loops to ensure the button state is updated on 30 fps game
            vm->Step();
            vm->Step();
            bool btnbits = vm->ExecuteLua(
                "function btnbitstest1()\n"
                " return btnbits == 1\n"
                "end\n",
                "btnbitstest1");

            CHECK(btnbits);
        }
        SUBCASE("right pushed returns 2"){
            stubHost->stubInput(2, 2);
            //requires two loops to ensure the button state is updated on 30 fps game
            vm->Step();
            vm->Step();
            bool btnbits = vm->ExecuteLua(
                "function btnbitstest2()\n"
                " return btnbits == 2\n"
                "end\n",
                "btnbitstest2");

            CHECK(btnbits);
        }
        SUBCASE("right pushed detected by band op"){
            stubHost->stubInput(2, 2);
            //requires two loops to ensure the button state is updated on 30 fps game
            vm->Step();
            vm->Step();
            bool btnbits = vm->ExecuteLua(
                "function rightbandtest()\n"
                " return rightband == 2\n"
                "end\n",
                "rightbandtest");

            CHECK(btnbits);
        }
        vm->CloseCart();
    }

    SUBCASE("emoji button variables are available in sandbox"){
        vm->LoadCart("emojibuttons.p8");
        vm->vm_run();
        vm->Step();

        SUBCASE("emoji button variables resolve to correct numeric values"){
            // Test that ⬅️ = 0, ➡️ = 1, ⬆️ = 2, ⬇️ = 3, 🅾️ = 4, ❎ = 5
            bool left_correct = vm->ExecuteLua(
                "function test_left_val()\n"
                " return left_val == 0\n"
                "end\n",
                "test_left_val");
            CHECK(left_correct);

            bool right_correct = vm->ExecuteLua(
                "function test_right_val()\n"
                " return right_val == 1\n"
                "end\n",
                "test_right_val");
            CHECK(right_correct);

            bool up_correct = vm->ExecuteLua(
                "function test_up_val()\n"
                " return up_val == 2\n"
                "end\n",
                "test_up_val");
            CHECK(up_correct);

            bool down_correct = vm->ExecuteLua(
                "function test_down_val()\n"
                " return down_val == 3\n"
                "end\n",
                "test_down_val");
            CHECK(down_correct);

            bool o_correct = vm->ExecuteLua(
                "function test_o_val()\n"
                " return o_val == 4\n"
                "end\n",
                "test_o_val");
            CHECK(o_correct);

            bool x_correct = vm->ExecuteLua(
                "function test_x_val()\n"
                " return x_val == 5\n"
                "end\n",
                "test_x_val");
            CHECK(x_correct);
        }

        SUBCASE("btn with emoji argument checks correct button"){
            // Press O button (bit 4 = 16) and verify btn(🅾️) returns true but btn(❎) returns false
            stubHost->stubInput(16, 16); // O button pressed
            vm->Step();
            vm->Step();

            bool o_pressed = vm->ExecuteLua(
                "function test_btn_o()\n"
                " return btn_o == true\n"
                "end\n",
                "test_btn_o");
            CHECK(o_pressed);

            bool x_not_pressed = vm->ExecuteLua(
                "function test_btn_x_not()\n"
                " return btn_x == false\n"
                "end\n",
                "test_btn_x_not");
            CHECK(x_not_pressed);

            bool left_not_pressed = vm->ExecuteLua(
                "function test_btn_left_not()\n"
                " return btn_left == false\n"
                "end\n",
                "test_btn_left_not");
            CHECK(left_not_pressed);
        }

        SUBCASE("left button does not trigger O or X button checks"){
            // Press left button (bit 0 = 1) and verify btn(🅾️) and btn(❎) return false
            stubHost->stubInput(1, 1); // Left button pressed
            vm->Step();
            vm->Step();

            bool left_pressed = vm->ExecuteLua(
                "function test_left_pressed()\n"
                " return btn_left == true\n"
                "end\n",
                "test_left_pressed");
            CHECK(left_pressed);

            bool o_not_pressed = vm->ExecuteLua(
                "function test_o_not_pressed()\n"
                " return btn_o == false\n"
                "end\n",
                "test_o_not_pressed");
            CHECK(o_not_pressed);

            bool x_not_pressed = vm->ExecuteLua(
                "function test_x_not_pressed()\n"
                " return btn_x == false\n"
                "end\n",
                "test_x_not_pressed");
            CHECK(x_not_pressed);
        }

        vm->CloseCart();
    }

    SUBCASE("togglepausemenu resets and restores draw state") {
        graphics->pal(10, 12, 0);
        graphics->fillp(25);
        graphics->clip(11, 12, 21, 22);
        graphics->camera(1, 2);
        graphics->color(3);

        SUBCASE("togglepausemenu resets draw state"){
            vm->togglePauseMenu();
            
            CHECK_EQ(10, graphics->getDrawPalMappedColor(10));

            CHECK_EQ(0, memory->drawState.fillPattern[0]);

            CHECK_EQ(0, memory->drawState.clip_xb);
            CHECK_EQ(0, memory->drawState.clip_yb);
            CHECK_EQ(128, memory->drawState.clip_xe);
            CHECK_EQ(128, memory->drawState.clip_ye);

            CHECK_EQ(0, memory->drawState.camera_x);
            CHECK_EQ(0, memory->drawState.camera_y);

            CHECK_EQ(6, memory->drawState.color);
        }
        SUBCASE("togglepausemenu second time restores draw state"){
            vm->togglePauseMenu();
            vm->togglePauseMenu();
            
            CHECK_EQ(12, graphics->getDrawPalMappedColor(10));

            CHECK_EQ(25, memory->drawState.fillPattern[0]);

            CHECK_EQ(11, memory->drawState.clip_xb);
            CHECK_EQ(12, memory->drawState.clip_yb);
            CHECK_EQ(32, memory->drawState.clip_xe);
            CHECK_EQ(34, memory->drawState.clip_ye);

            CHECK_EQ(1, memory->drawState.camera_x);
            CHECK_EQ(2, memory->drawState.camera_y);

            CHECK_EQ(3, memory->drawState.color);
        }
    }

    SUBCASE("getSerializedCartData for all zeroes"){
        vm->vm_cartdata("serializeTest");
        for(int i = 0; i < 256; i++) {
            memory->cartData[i] = 0;
        }

        auto result = vm->getSerializedCartData();

        std::string expected = 
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n";

        CHECK_EQ(expected, result);
    }
    SUBCASE("getSerializedCartData with 0-255"){
        vm->vm_cartdata("serializeTest");
        for(int i = 0; i < 256; i++) {
            memory->cartData[i] = (uint8_t)i;
        }

        auto result = vm->getSerializedCartData();

        std::string expected = 
            "03020100070605040b0a09080f0e0d0c13121110171615141b1a19181f1e1d1c\n"
            "23222120272625242b2a29282f2e2d2c33323130373635343b3a39383f3e3d3c\n"
            "43424140474645444b4a49484f4e4d4c53525150575655545b5a59585f5e5d5c\n"
            "63626160676665646b6a69686f6e6d6c73727170777675747b7a79787f7e7d7c\n"
            "83828180878685848b8a89888f8e8d8c93929190979695949b9a99989f9e9d9c\n"
            "a3a2a1a0a7a6a5a4abaaa9a8afaeadacb3b2b1b0b7b6b5b4bbbab9b8bfbebdbc\n"
            "c3c2c1c0c7c6c5c4cbcac9c8cfcecdccd3d2d1d0d7d6d5d4dbdad9d8dfdedddc\n"
            "e3e2e1e0e7e6e5e4ebeae9e8efeeedecf3f2f1f0f7f6f5f4fbfaf9f8fffefdfc\n";

        CHECK_EQ(expected, result);
    }
    SUBCASE("deserializeCartDataToMemory for all zeroes"){
        vm->vm_cartdata("serializeTest");
        for(int i = 0; i < 256; i++) {
            memory->cartData[i] = 1;
        }

        std::string allZeroes = 
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n";

        vm->deserializeCartDataToMemory(allZeroes);

        bool matches = true;
        for(int i = 0; i < 256; i++) {
            matches &= memory->cartData[i] == 0;
        }

        CHECK(matches);
    }
    SUBCASE("deserializeCartDataToMemory with 0-255"){
        vm->vm_cartdata("serializeTest");
        for(int i = 0; i < 256; i++) {
            memory->cartData[i] = 1;
        }

        std::string values = 
            "03020100070605040b0a09080f0e0d0c13121110171615141b1a19181f1e1d1c\n"
            "23222120272625242b2a29282f2e2d2c33323130373635343b3a39383f3e3d3c\n"
            "43424140474645444b4a49484f4e4d4c53525150575655545b5a59585f5e5d5c\n"
            "63626160676665646b6a69686f6e6d6c73727170777675747b7a79787f7e7d7c\n"
            "83828180878685848b8a89888f8e8d8c93929190979695949b9a99989f9e9d9c\n"
            "a3a2a1a0a7a6a5a4abaaa9a8afaeadacb3b2b1b0b7b6b5b4bbbab9b8bfbebdbc\n"
            "c3c2c1c0c7c6c5c4cbcac9c8cfcecdccd3d2d1d0d7d6d5d4dbdad9d8dfdedddc\n"
            "e3e2e1e0e7e6e5e4ebeae9e8efeeedecf3f2f1f0f7f6f5f4fbfaf9f8fffefdfc\n";

        vm->deserializeCartDataToMemory(values);

        bool matches = true;
        for(int i = 0; i < 256; i++) {
            matches &= memory->cartData[i] == i;
        }

        CHECK(matches);
    }
    SUBCASE("dset then getSerializedCartData returns correct values"){
        vm->vm_cartdata("serializeTest");
        vm->vm_dset(32, 128);
        vm->vm_dset(0, 1);
        vm->vm_dset(1, 2);

        auto actual = vm->getSerializedCartData();

        std::string expected = 
            "0001000000020000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0080000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n";

        CHECK_EQ(expected, actual);
    }
    SUBCASE("dset then getSerializedCartData with negatives returns correct values"){
        vm->vm_cartdata("serializeTest");
        vm->vm_dset(0, -1);
	    vm->vm_dset(1, 1);
	    vm->vm_dset(2, 32767);
	    vm->vm_dset(3, -32768);

        auto actual = vm->getSerializedCartData();

        std::string expected = 
            "ffff0000000100007fff00008000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n";

        CHECK_EQ(expected, actual);
    }
    SUBCASE("multiple cartdata keys can be used (up to 4)"){
        vm->vm_cartdata("key1");
        vm->vm_dset(0, 100);
        vm->vm_cartdata("key2");
        vm->vm_cartdata("key3");
        vm->vm_cartdata("key4");
        vm->vm_dset(0, 400);

        CHECK_EQ(vm->vm_dget(0), (fix32)400);
        
        vm->vm_cartdata("key2");
        CHECK_EQ(vm->vm_dget(0), (fix32)0);
        
        vm->vm_dset(0, 200);
        CHECK_EQ(vm->vm_dget(0), (fix32)200);
        
        vm->vm_cartdata("key1");
        CHECK_EQ(vm->vm_dget(0), (fix32)100);
    }
    SUBCASE("error when trying to use more than 4 cartdata keys"){
        vm->vm_cartdata("key1");
        vm->vm_cartdata("key2");
        vm->vm_cartdata("key3");
        vm->vm_cartdata("key4");
        
        // 5th key should fail
        CHECK_FALSE(vm->vm_cartdata("key5"));
        CHECK_EQ(vm->GetBiosError(), "too many cart data keys (max 4)");
    }
    SUBCASE("switching between different cartdata keys works correctly"){
        vm->vm_cartdata("save1");
        vm->vm_dset(0, 111);
        vm->vm_dset(1, 222);
        
        vm->vm_cartdata("save2");
        vm->vm_dset(0, 333);
        vm->vm_dset(1, 444);
        
        vm->vm_cartdata("save1");
        CHECK_EQ(vm->vm_dget(0), (fix32)111);
        CHECK_EQ(vm->vm_dget(1), (fix32)222);
        
        vm->vm_cartdata("save2");
        CHECK_EQ(vm->vm_dget(0), (fix32)333);
        CHECK_EQ(vm->vm_dget(1), (fix32)444);
    }
    SUBCASE("reusing same cartdata key multiple times works"){
        // Use the same key multiple times - should work fine
        vm->vm_cartdata("sharedkey");
        vm->vm_dset(0, 555);
        
        vm->vm_cartdata("sharedkey");
        CHECK_EQ(vm->vm_dget(0), (fix32)555);
        
        vm->vm_dset(1, 666);
        
        vm->vm_cartdata("sharedkey");
        CHECK_EQ(vm->vm_dget(0), (fix32)555);
        CHECK_EQ(vm->vm_dget(1), (fix32)666);
    }
    SUBCASE("backward compatibility - single key usage still works"){
        vm->vm_cartdata("singlekey");
        vm->vm_dset(0, 777);
        vm->vm_dset(1, 888);
        
        CHECK_EQ(vm->vm_dget(0), (fix32)777);
        CHECK_EQ(vm->vm_dget(1), (fix32)888);
        
        vm->vm_cartdata("singlekey");
        CHECK_EQ(vm->vm_dget(0), (fix32)777);
        CHECK_EQ(vm->vm_dget(1), (fix32)888);
    }
    SUBCASE("current active key is tracked correctly for serialization"){
        vm->vm_cartdata("active1");
        vm->vm_dset(0, 999);
        vm->vm_dset(1, 1111);
        
        vm->vm_cartdata("active2");
        vm->vm_dset(0, 2222);
        vm->vm_dset(1, 3333);
        
        vm->vm_cartdata("active1");
        CHECK_EQ(vm->vm_dget(0), (fix32)999);
        CHECK_EQ(vm->vm_dget(1), (fix32)1111);

        vm->vm_dset(0, 4444);
        vm->vm_dset(1, 5555);
        
        auto serialized = vm->getSerializedCartData();
        
        // 4444 = 0x115C, 5555 = 0x15B3 (in little-endian format)
        std::string expected = 
            "115c000015b30000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n";
        
        CHECK_EQ(expected, serialized);
    }
    SUBCASE("deserializeCartDataToMemory with negative ints"){
        vm->vm_cartdata("serializeTest");
        for(int i = 0; i < 256; i++) {
            memory->cartData[i] = 0;
        }

        std::string values = 
            "ffff0000000100007fff00008000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n"
            "0000000000000000000000000000000000000000000000000000000000000000\n";

        vm->deserializeCartDataToMemory(values);

        CHECK_EQ(vm->vm_dget(0), (fix32)-1);
        CHECK_EQ(vm->vm_dget(1), (fix32)1);
        CHECK_EQ(vm->vm_dget(2), (fix32)32767);
        CHECK_EQ(vm->vm_dget(3), (fix32)-32768);
    }
    SUBCASE("SFX Note getters match expected values"){
        memory->sfx[0].notes[0].data[0] = 205;
        memory->sfx[0].notes[0].data[1] = 233;

        CHECK(memory->sfx[0].notes[0].getWaveform() == 7);
        CHECK(memory->sfx[0].notes[0].getEffect() == 6);
        CHECK(memory->sfx[0].notes[0].getKey() == 13);
        CHECK(memory->sfx[0].notes[0].getVolume() == 4);
        CHECK(memory->sfx[0].notes[0].getCustom() == 1);
    }
    SUBCASE("SFX Note settiers set correct values"){
        memory->sfx[0].notes[0].setWaveform(7);
        memory->sfx[0].notes[0].setEffect(6);
        memory->sfx[0].notes[0].setKey(13);
        memory->sfx[0].notes[0].setVolume(4);
        memory->sfx[0].notes[0].setCustom(1);

        CHECK(memory->sfx[0].notes[0].data[0] == 205);
        CHECK(memory->sfx[0].notes[0].data[1] == 233);
    }
    SUBCASE("cartlist gets sorted"){
        std::vector<std::string> vec = { "Qcart.p8", "Acart.p8", "Zcart.p8.png", "Jcart.p8.png" };
        vm->SetCartList(vec);
        auto sorted = vm->GetCartList();

        CHECK_EQ("Acart.p8", sorted[0]);
        CHECK_EQ("Jcart.p8.png", sorted[1]);
        CHECK_EQ("Qcart.p8", sorted[2]);
        CHECK_EQ("Zcart.p8.png", sorted[3]);
    }
    SUBCASE("reset sets hw and draw state back to defaults"){
        graphics->pal(4, 5, 0);
        graphics->color(14);

        memory->hwState.widthOfTheMap = 17;
        memory->hwState.alternatePaletteFlag = 2;

        vm->vm_reset();

        CHECK_EQ(128, memory->hwState.widthOfTheMap);
        CHECK_EQ(6, memory->drawState.color);
        CHECK_EQ(4, graphics->getDrawPalMappedColor(4));
        CHECK_EQ(0, memory->hwState.alternatePaletteFlag);
    }
    SUBCASE("pause menu - initial state is not paused") {
        CHECK(vm->IsPaused() == false);
    }
    SUBCASE("pause menu - togglePauseMenu toggles state") {
        CHECK(vm->IsPaused() == false);
        vm->togglePauseMenu();
        CHECK(vm->IsPaused() == true);
        vm->togglePauseMenu();
        CHECK(vm->IsPaused() == false);
    }

    delete stubHost;
    delete graphics;
    delete input;
    delete audio;

    delete vm;

    delete memory;
}

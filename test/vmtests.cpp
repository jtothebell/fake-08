#include <string>
#include <vector>

#include "doctest.h"

#include "../source/vm.h"
#include "../source/hostVmShared.h"

#include "../source/fontdata.h"


TEST_CASE("Vm memory functions") {
    PicoRam* memory = new PicoRam();
    Graphics* graphics = new Graphics(get_font_data(), memory);
    Input* input = new Input();
    Audio* audio = new Audio(memory);

    Vm* vm = new Vm(memory, graphics, input, audio);

    SUBCASE("memory data stats with 0"){
        CHECK(memory->data[0] == 0);
    }
    SUBCASE("size of PicoRam struct corrct"){
        CHECK(sizeof(PicoRam) == 0x8000);
    }
    SUBCASE("size of data correct"){
        CHECK(sizeof(memory->data) == 0x8000);
    }
    SUBCASE("simple peek and poke"){
        vm->ram_poke(2, 232);

        CHECK(vm->ram_peek(2) == 232);
    }
    SUBCASE("simple peek2 and poke2"){
        vm->ram_poke2(41, -1031);

        CHECK(vm->ram_peek2(41) == -1031);
    }
    SUBCASE("simple peek4 and poke4"){
        vm->ram_poke4(0x7123, 49249);

        CHECK(vm->ram_peek4(0x7123) == 49249);
    }
    SUBCASE("poking spritesheet"){
        //147: 1001 0011
        //left pixel: 0011 = 3
        //right pixel: 1001 = 9
        vm->ram_poke(0, 147);

        CHECK_EQ(graphics->sget(0, 0), 3);
        CHECK_EQ(graphics->sget(1, 0), 9);
    }
    SUBCASE("poking map"){
        vm->ram_poke(0x2000, 201);
        vm->ram_poke(0x2003, 11);

        CHECK_EQ(graphics->mget(0, 0), 201);
        CHECK_EQ(graphics->mget(3, 0), 11);
    }
    SUBCASE("poking sprite flags"){
        //39: 0010 0111
        vm->ram_poke(0x3000, 39);

        CHECK_EQ(graphics->fget(0), 39);
        CHECK_EQ(graphics->fget(0, 0), true);
        CHECK_EQ(graphics->fget(0, 3), false);
    }
    SUBCASE("poking music"){
        //78: 0100 1110
        vm->ram_poke(0x3100, 78);

        CHECK_EQ(memory->songs[0].sfx0, 78);
        CHECK_EQ(memory->songs[0].start, 0);
    }
    SUBCASE("poking sfx"){
        //97: 0110 0001
        //key is first 6 bits: 10 0001 : 33
        //waveform is next 3: (0) 01 : 1
        vm->ram_poke(0x3200, 97);

        CHECK_EQ(memory->sfx[0].notes[0].key, 33);
        CHECK_EQ(memory->sfx[0].notes[0].waveform, 1);
    }
    SUBCASE("poking general use ram"){
        vm->ram_poke(0x4300, 215);

        CHECK_EQ(memory->generalUseRam[0], 215);
    }
    SUBCASE("setting cart data"){
        vm->vm_cartdata("dummy");
        vm->vm_dset(13, 56);

        CHECK_EQ(vm->vm_dget(13), 56);
    }
    SUBCASE("poking cart data"){
        vm->ram_poke(0x5e00, 56);
        vm->vm_cartdata("dummy");

        CHECK_EQ(memory->cartData[0], 56);
        CHECK_EQ(vm->vm_dget(0), 56);
    }
    
    delete graphics;
    delete input;
    delete audio;

    delete vm;

    delete memory;
}
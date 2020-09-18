#include <string>
#include <vector>

#include "doctest.h"

#include "../source/vm.h"
#include "../source/hostVmShared.h"


TEST_CASE("Vm memory functions") {
    Vm* vm = new Vm();
    PicoRam* memory = vm->getPicoRam();

    SUBCASE("memory data stats with 0"){
        CHECK(memory->data[0] == 0);
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
    

    delete vm;
}
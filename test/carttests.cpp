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

}
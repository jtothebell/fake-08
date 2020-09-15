#include <string>

#include "doctest.h"
#include "../source/cartPatcher.h"

TEST_CASE("Cart Patcher/Preprocessor tests") {

    SUBCASE("patching lua handles empty string"){
        std::string patched = getPatchedLua("");

        CHECK_EQ(patched.length(), 0);
    }
    SUBCASE("assignment operators expanded"){
        std::string operators[] = {
            "+", "-", "*", "/", "\\", "%", "^", "..",
            "|", "&", "^^", "<<", ">>", ">>>"
        };

        for (int i = 0; i < 14; i++) {
            std::string unpatched = "foo" + operators[i] +  "=bar";
            std::string patched = getPatchedLua(unpatched.c_str());

            CHECK(patched == "foo = foo " + operators[i] + " bar");
        };
    }
    SUBCASE("!= converted"){
        std::string patched = getPatchedLua("thing != other");

        CHECK(patched == "thing ~= other");
    }
    SUBCASE("// c style comment converted"){
        std::string patched = getPatchedLua("//this is a comment");

        CHECK(patched == "--this is a comment");
    }
    SUBCASE("shorthand if statement converted"){
        std::string patched = getPatchedLua("if (pl.drilling>0) pl.drilling-=1\n");

        CHECK(patched == "if pl.drilling>0 then pl.drilling = pl.drilling - 1 end\n");

        patched = getPatchedLua("if (thingIsTrue()) doADifferentThing()\n");

        CHECK(patched == "if thingIsTrue() then doADifferentThing() end\n");
    }
    SUBCASE("shorthand if else statement converted"){
        std::string patched = getPatchedLua("if (cond2) print(\"cond2\") else print(\"not cond2\")\n");

        CHECK(patched == "if cond2 then print(\"cond2\") else print(\"not cond2\") end\n");
    }
    SUBCASE("shorthand while statement converted"){
        std::string patched = getPatchedLua("x=0 while(x<5) print(x) x+=1\n");

        CHECK(patched == "x=0 while (x<5) do print(x) x = x + 1 end\n");
    }

}

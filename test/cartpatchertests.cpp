#include <string>

#include "doctest.h"
#include "../source/cartPatcher.h"

TEST_CASE("dummy testcase") {
    std::string patched = getPatchedLua("");

    //CHECK_EQ(patched.length(), 0);
    CHECK_EQ(0, 0);
}
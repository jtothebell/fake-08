#include "doctest.h"
#include "../source/nibblehelpers.h"

TEST_CASE("Get Combined pixel index") {
    SUBCASE("0,0") {
        CHECK_EQ(getCombinedIdx(0, 0), 0);
    }
    SUBCASE("1,0") {
        CHECK_EQ(getCombinedIdx(1, 0), 0);
    }
    SUBCASE("127, 127") {
        CHECK_EQ(getCombinedIdx(127, 127), 8191);
    }
    SUBCASE("126, 127") {
        CHECK_EQ(getCombinedIdx(127, 127), 8191);
    }
    SUBCASE("37, 99") {
        CHECK_EQ(getCombinedIdx(37, 99), 6354);
    }
}

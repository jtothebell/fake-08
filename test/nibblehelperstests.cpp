#include "doctest.h"
#include "../source/nibblehelpers.h"

TEST_CASE("Get Combined pixel index") {
    SUBCASE("getCombinedIdx 0,0") {
        CHECK_EQ(getCombinedIdx(0, 0), 0);
    }
    SUBCASE("getCombinedIdx 1,0") {
        CHECK_EQ(getCombinedIdx(1, 0), 0);
    }
    SUBCASE("getCombinedIdx 127, 127") {
        CHECK_EQ(getCombinedIdx(127, 127), 8191);
    }
    SUBCASE("getCombinedIdx 126, 127") {
        CHECK_EQ(getCombinedIdx(127, 127), 8191);
    }
    SUBCASE("getCombinedIdx 37, 99") {
        CHECK_EQ(getCombinedIdx(37, 99), 6354);
    }
    SUBCASE("isValidSprIdx 0,0 true") {
        CHECK(isValidSprIdx(0, 0));
    }
    SUBCASE("isValidSprIdx 1,0 true") {
        CHECK(isValidSprIdx(1, 0));
    }
    SUBCASE("isValidSprIdx 127, 127 true") {
        CHECK(isValidSprIdx(127, 127));
    }
    SUBCASE("isValidSprIdx -1,0 false") {
        CHECK_FALSE(isValidSprIdx(-1, 0));
    }
    SUBCASE("isValidSprIdx 128, 127 false") {
        CHECK_FALSE(isValidSprIdx(128, 127));
    }
    SUBCASE("isValidSprIdx 127, 128 false") {
        CHECK_FALSE(isValidSprIdx(127, 128));
    }
}

#include "doctest.h"
#include "../source/fontdata.h"

TEST_CASE("checking fontdata exists") {
    //test failing test
    CHECK(get_font_data().length() == 15996);
}
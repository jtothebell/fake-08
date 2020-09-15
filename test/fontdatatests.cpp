#include "doctest.h"
#include "../source/fontdata.h"

TEST_CASE("checking fontdata exists") {
    CHECK(get_font_data().length() == 15995);
}
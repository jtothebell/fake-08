#include "doctest.h"
#include "../source/filehelpers.h"

TEST_CASE("isAbsolutePath") {
    SUBCASE("empty string") {
        CHECK_EQ(isAbsolutePath(""), false);
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(isAbsolutePath("file"), false);
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(isAbsolutePath("cart.p8"), false);
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(isAbsolutePath("dir/cart.p8"), false);
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(isAbsolutePath("/dir1/dir2/cart.p8"), true);
    }
    SUBCASE("vita path") {
        CHECK_EQ(isAbsolutePath("ux0:/data/dir2/cart.p8"), true);
    }
}

TEST_CASE("getDirectory") {
    SUBCASE("empty string") {
        CHECK_EQ(getDirectory(""), "");
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(getDirectory("file"), "");
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(getDirectory("cart.p8"), "");
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(getDirectory("dir/cart.p8"), "dir");
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(getDirectory("/dir1/dir2/cart.p8"), "/dir1/dir2");
    }
    SUBCASE("vita path") {
        CHECK_EQ(getDirectory("ux0:/data/dir2/cart.p8"), "ux0:/data/dir2");
    }
}

TEST_CASE("getFileExtension") {
    SUBCASE("empty string") {
        CHECK_EQ(getFileExtension(""), "");
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(getFileExtension("file"), "");
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(getFileExtension("cart.p8"), ".p8");
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(getFileExtension("dir/cart.p8"), ".p8");
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(getFileExtension("/dir1/dir2/cart.p8.png"), ".png");
    }
    SUBCASE("vita path") {
        CHECK_EQ(getFileExtension("ux0:/data/di.r2/ca.rt.p8.longer"), ".longer");
    }
}

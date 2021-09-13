#include "doctest.h"
#include "../source/filehelpers.h"

TEST_CASE("isAbsolutePath") {
    SUBCASE("empty string") {
        CHECK_EQ(filehelpers::isAbsolutePath(""), false);
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(filehelpers::isAbsolutePath("file"), false);
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(filehelpers::isAbsolutePath("cart.p8"), false);
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(filehelpers::isAbsolutePath("dir/cart.p8"), false);
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(filehelpers::isAbsolutePath("/dir1/dir2/cart.p8"), true);
    }
    SUBCASE("vita path") {
        CHECK_EQ(filehelpers::isAbsolutePath("ux0:/data/dir2/cart.p8"), true);
    }
}

TEST_CASE("getDirectory") {
    SUBCASE("empty string") {
        CHECK_EQ(filehelpers::getDirectory(""), "");
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(filehelpers::getDirectory("file"), "");
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(filehelpers::getDirectory("cart.p8"), "");
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(filehelpers::getDirectory("dir/cart.p8"), "dir");
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(filehelpers::getDirectory("/dir1/dir2/cart.p8"), "/dir1/dir2");
    }
    SUBCASE("vita path") {
        CHECK_EQ(filehelpers::getDirectory("ux0:/data/dir2/cart.p8"), "ux0:/data/dir2");
    }
}

TEST_CASE("getFileExtension") {
    SUBCASE("empty string") {
        CHECK_EQ(filehelpers::getFileExtension(""), "");
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(filehelpers::getFileExtension("file"), "");
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(filehelpers::getFileExtension("cart.p8"), ".p8");
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(filehelpers::getFileExtension("dir/cart.p8"), ".p8");
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(filehelpers::getFileExtension("/dir1/dir2/cart.p8.png"), ".png");
    }
    SUBCASE("vita path") {
        CHECK_EQ(filehelpers::getFileExtension("ux0:/data/di.r2/ca.rt.p8.longer"), ".longer");
    }
}

TEST_CASE("isHidden") {
    SUBCASE("empty string") {
        CHECK_EQ(filehelpers::isHiddenFile(""), false);
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(filehelpers::isHiddenFile("file"), false);
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(filehelpers::isHiddenFile("cart.p8"), false);
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(filehelpers::isHiddenFile("dir/cart.p8"), false);
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(filehelpers::isHiddenFile("/dir1/dir2/cart.p8"), false);
    }
    SUBCASE("vita path") {
        CHECK_EQ(filehelpers::isHiddenFile("ux0:/data/dir2/cart.p8"), false);
    }
    SUBCASE("hidden file") {
        CHECK_EQ(filehelpers::isHiddenFile(".bash_rc"), false);
    }
    SUBCASE("macOS filesystem thing") {
        CHECK_EQ(filehelpers::isHiddenFile("._cart.p8"), true);
    }
}

TEST_CASE("isCartFile") {
    SUBCASE("empty string") {
        CHECK_EQ(filehelpers::isCartFile(""), false);
    }
    SUBCASE("filename no etension") {
        CHECK_EQ(filehelpers::isCartFile("file"), false);
    }
    SUBCASE("filename with etension") {
        CHECK_EQ(filehelpers::isCartFile("cart.p8"), true);
    }
    SUBCASE("filename with extension in folder") {
        CHECK_EQ(filehelpers::isCartFile("dir/cart.p8"), true);
    }
    SUBCASE("path starting with slash") {
        CHECK_EQ(filehelpers::isCartFile("/dir1/dir2/cart.p8.png"), true);
    }
    SUBCASE("vita path") {
        CHECK_EQ(filehelpers::isCartFile("ux0:/data/dir2/cart.p8"), true);
    }
    SUBCASE("hidden file") {
        CHECK_EQ(filehelpers::isCartFile(".bash_rc"), false);
    }
    SUBCASE("macOS filesystem thing p8") {
        CHECK_EQ(filehelpers::isCartFile("._cart.p8"), false);
    }
    SUBCASE("macOS filesystem thing png") {
        CHECK_EQ(filehelpers::isCartFile("._cart.p8.png"), false);
    }
}

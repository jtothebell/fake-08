#include "doctest.h"
#include "doctest.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include <string>

TEST_CASE("shortprint (?)") {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    //define a fake print function - returns the args (note this is different
    //from Pico-8 which returns the new x value)
    luaL_dostring(L, "function print(...) return ... end");

    SUBCASE("shortprint as assignment") {
        const char* code = 
        "local a= ?'test'\n"
        "return a";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TSTRING);
            CHECK_EQ(std::string(lua_tostring(L, -1)), "test");
        }
    }

    SUBCASE("shortprint as return") {
        const char* code = "return ?'ret'";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
             CHECK_EQ(lua_type(L, -1), LUA_TSTRING);
             CHECK_EQ(std::string(lua_tostring(L, -1)), "ret");
        }
    }

    SUBCASE("shortprint nested") {
        const char* code = 
        "local a = ?('nested')\n"
        "return a";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(std::string(lua_tostring(L, -1)), "nested");
        }
    }

    lua_close(L);
}

//IF (NOT B) I=1 J=2
TEST_CASE("IF and WHILE shorthand") {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("short if") {
        const char* code = 
        "b=false\n"
        "if (not b) i=1 j=2\n"
        "return j\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 2);
        }
    }

    SUBCASE("short while") {
        const char* code = 
        "x=5\n"
        "y=4\n"
        "while (x<7) x=x+1 y=y*2\n"
        "return y\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 16);
        }
    }

    lua_close(L);
}

//"+=", "-=", "*=", "/=", "%=", "^=", "\\=", "&=", "|=",
//    "^^=", "<<=", ">>=", ">>>=", "<<>=", ">><=", "..=",
TEST_CASE("Assignment operators") {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("addition assignment (+=)") {
        const char* code = 
        "x=6\n"
        "x+=10\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 16);
        }
    }
    SUBCASE("subtraction assignment (-=)") {
        const char* code = 
        "x=6\n"
        "x-=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 3);
        }
    }
    SUBCASE("multiplication assignment (*=)") {
        const char* code = 
        "x=6\n"
        "x*=7\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 42);
        }
    }
    SUBCASE("division assignment (*=)") {
        const char* code = 
        "x=6\n"
        "x/=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 2);
        }
    }
    SUBCASE("mod assignment (%=)") {
        const char* code = 
        "x=6\n"
        "x%=5\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 1);
        }
    }
    SUBCASE("pow assignment (^=)") {
        const char* code = 
        "x=6\n"
        "x^=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 216);
        }
    }
    SUBCASE("int division assignment (\\=)") {
        const char* code = 
        "x=33\n"
        "x\\=8\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 4);
        }
    }
    SUBCASE("bitwise and assignment (&=)") {
        const char* code = 
        "x=10\n"
        "x&=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 2);
        }
    }
    SUBCASE("bitwise or assignment (|=)") {
        const char* code = 
        "x=10\n"
        "x|=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 11);
        }
    }
    SUBCASE("bitwise xor assignment (^^=)") {
        const char* code = 
        "x=3\n"
        "x^^=5\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 6);
        }
    }
    SUBCASE("bitwise shift left assignment (<<=)") {
        const char* code = 
        "x=5\n"
        "x<<=2\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 20);
        }
    }
    SUBCASE("bitwise shift right assignment (>>=)") {
        const char* code = 
        "x=210\n"
        "x>>=2\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 52);
        }
    }
    SUBCASE("logical shift right assignment (>>>=)") {
        const char* code = 
        "x=12\n"
        "x>>>=2\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 3);
        }
    }
    SUBCASE("rotate left assignment (<<>=)") {
        const char* code = 
        "x=11\n"
        "x<<>=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 88);
        }
    }
    SUBCASE("rotate right assignment (>><=)") {
        const char* code = 
        "x=480\n"
        "x>><=3\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 60);
        }
    }
    SUBCASE("concat assignment operator (..=)") {
        const char* code = 
        "x='hello'\n"
        "x..='world'\n"
        "return x\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TSTRING);
            CHECK_EQ(std::string(lua_tostring(L, -1)), "helloworld");
        }
    }

    
    lua_close(L);
}

TEST_CASE("Alternative xor operator") {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("alt xor (~)") {
        const char* code = 
        "return 10~2";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 8);
        }
    }

    lua_close(L);
}

TEST_CASE("Alternative not equal (!=)") {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("alt not equal (!=)") {
        const char* code = 
        "return 10!=2";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TBOOLEAN);
            CHECK_EQ(lua_toboolean(L, -1), true);
        }
    }

    lua_close(L);
}
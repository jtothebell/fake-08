#include "doctest.h"
#include "doctest.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include <string>

TEST_CASE("shortprint (?)") {
    lua_State* L = luaL_newstate();
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
    lua_State* L = luaL_newstate();
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

    SUBCASE("nested short while with short if in outer while") {
        const char* code =
            "local t,e=3,\"abcdefghi\"\n"
            "local o,n,f=3,1,8\n"
            "while t<#e do\n"
            "while(o>0)o-=1n+=1if(n>f)n=1\n"
            "t+=2 end\n"
            "return t\n";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_type(L, -1), LUA_TNUMBER);
            CHECK_EQ(lua_tointeger(L, -1), 9);
        }
    }

    lua_close(L);
}

//"+=", "-=", "*=", "/=", "%=", "^=", "\\=", "&=", "|=",
//    "^^=", "<<=", ">>=", ">>>=", "<<>=", ">><=", "..=",
TEST_CASE("Assignment operators") {
    lua_State* L = luaL_newstate();
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
    lua_State* L = luaL_newstate();
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
    lua_State* L = luaL_newstate();
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
TEST_CASE("Nested Shorthand IF logic") {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("shorthand if with shortprint swallowing next line") {
        const char* code =
            "res=0 c=2\n"
            "if c==1 then\n"
            "  if (1==1) ? 'test'\n"
            "elseif c==2 then\n"
            "  res=2\n"
            "end\n"
            "return res";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            // Prior to a bug fix in lparser.c, the 'elseif' is swallowed by the inner 'if',
            // resulting in res remaining 0.
            CHECK_EQ(lua_tointeger(L, -1), 2);
        }
    }

    SUBCASE("shorthand if with else on same line") {
        const char* code =
            "if (1==0) return 1 else return 2";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 2);
        }
    }

    SUBCASE("shorthand if nested with outer else if (same line recursion)") {
        const char* code =
            "res=0 if(1==0) res=1 else if(1==1) res=2\n"
            "return res";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 2);
        }
    }

    lua_close(L);
}

TEST_CASE("g collision param d shadows global d with entity tables") {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    const char* code =
        "function d() return \"memcpy\" end\n"
        "function g(d,n,e)local f,a,o,e,c=e or 1,n.e-d.e,n.d-d.d,1,(d.dd or 5)+(n.dd or 5) return a end\n"
        "function memcpy() end\n"
        "local b1={e=104,d=200,i=0,o=0,a=0,dd=5}\n"
        "local i1={e=596,d=300,f=true,dd=5,i=0,o=0,a=0,c={t=\"red\"},u=d,d4=d}\n"
        "return g(b1,i1,.3)";

    int result = luaL_dostring(L, code);
    CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
    if (result == LUA_OK) {
        CHECK_EQ(lua_tointeger(L, -1), 492);
    }

    lua_close(L);
}

TEST_CASE("g collision param d shadows global d when entity lacks dd field") {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("global env") {
        const char* code =
            "function d() end\n"
            "function C(x,y,a,b)local x,y=(x-a)/64,(y-b)/64 return sqrt(x*x+y*y)*64 end\n"
            "function g(d,n,e)local f,a,o,e,c=e or 1,n.e-d.e,n.d-d.d,C(d.e,d.d,n.e,n.d),(d.dd or 5)+(n.dd or 5) return a end\n"
            "local b1={e=104,d=440,i=0,o=0,a=0}\n"
            "local i1={e=596,d=413,f=true,i=0,o=0,a=0,c={t=\"red\"}}\n"
            "return g(b1,i1,.3)";

        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 492);
        }
    }

    SUBCASE("sandbox env like cart") {
        const char* code =
            "local sandbox={}\n"
            "function sandbox.d() end\n"
            "function sandbox.C(x,y,a,b)local x,y=(x-a)/64,(y-b)/64 return sqrt(x*x+y*y)*64 end\n"
            "local chunk,err=load([[function g(d,n,e)local f,a,o,e,c=e or 1,n.e-d.e,n.d-d.d,C(d.e,d.d,n.e,n.d),(d.dd or 5)+(n.dd or 5) return a end]],nil,nil,sandbox)\n"
            "chunk()\n"
            "local b1={e=104,d=440,i=0,o=0,a=0,k=function()end,w=function(d)end}\n"
            "local i1={e=596,d=413,f=true,i=0,o=0,a=0,c={t=\"red\"}}\n"
            "return sandbox.g(b1,i1,.3)";

        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 492);
        }
    }

    lua_close(L);
}

TEST_CASE("Shorthand IF braces logic") {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("shorthand if with braces") {
        const char* code =
            "x=0 if (1==1) x=1\n"
            "return x";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 1);
        }
    }

    SUBCASE("shorthand if without braces") {
        const char* code =
            "x=0 if 1==1 x=1\n"
            "return x";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 1);
        }
    }

    lua_close(L);
}

TEST_CASE("if do keyword") {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    SUBCASE("do can be used instead of then") {
        const char* code =
            "local x=0\n"
            "if true do x=1 end\n"
            "return x";
        int result = luaL_dostring(L, code);
        CHECK_MESSAGE(result == LUA_OK, lua_tostring(L, -1));
        if (result == LUA_OK) {
            CHECK_EQ(lua_tointeger(L, -1), 1);
        }
    }

    lua_close(L);
}

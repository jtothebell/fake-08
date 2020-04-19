#include <string>

#include "cartPatcher.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

//from LovePotion: https://github.com/gamax92/picolove/blob/master/cart.lua (zlib license)
//it may be possible to do this kind of pattern matching with another library, or use z8Lua
//but for now I have lua 5.3 working so I'm going with this since I know it works


const char* getPatchedLua(const char* unpatchedLua){

    lua_State* L;

    // initialize Lua interpreter
    L = luaL_newstate();

    // load Lua base libraries (print / math / etc)
    luaL_openlibs(L);

    ////////////////////////////////////////////
    // We can use Lua here !
    //   Need access to the LuaState * variable L
    /////////////////////////////////////////////
    
    luaL_dofile(L, "cartPatcher.lua");

    // Push the fib function on the top of the lua stack
    lua_getglobal(L, "patchLuaForP8");

    // Push the argument (the number 13) on the stack 
    lua_pushstring(L, unpatchedLua);

    lua_pcall(L, 1, 1, 0);

    // Get the result from the lua stack
    const char* result = lua_tostring(L, -1);

    lua_pop(L,1);

    // Cleanup:  Deallocate all space assocatated with the lua state */
    lua_close(L);

    return result;

}



#include <string>

#include "picoluaapi.h"
#include "graphics.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

Graphics* _graphicsForLuaApi;

void initPicoApi(Graphics* graphics){
    _graphicsForLuaApi = graphics;
}

/*functions to expose to lua*/
int cls(lua_State *L){
    _graphicsForLuaApi->cls();

    return 0;
}
int pset(lua_State *L){
    double x = lua_tonumber(L,1);
    double y = lua_tonumber(L,2);
    double c = lua_tonumber(L,3);

    _graphicsForLuaApi->pset((short)x, (short)y, (uint8_t)c);

    return 0;
}
int pget(lua_State *L){
    double x = lua_tonumber(L,1);
    double y = lua_tonumber(L,2);

    uint8_t color = _graphicsForLuaApi->pget((short)x, (short)y);

    lua_pushinteger(L, color);

    return 1;
}
int color(lua_State *L){
    double c = lua_tonumber(L,1);

    _graphicsForLuaApi->color((uint8_t)c);

    return 0;
}
int line (lua_State *L){
    double x1 = lua_tonumber(L,1);
    double y1 = lua_tonumber(L,2);
    double x2 = lua_tonumber(L,3);
    double y2 = lua_tonumber(L,4);
    double c = lua_tonumber(L,5);

    _graphicsForLuaApi->line((short)x1, (short)y1, (short)x2, (short)y2, (uint8_t)c);

    return 0;
}
int circ(lua_State *L){
    double ox = lua_tonumber(L,1);
    double oy = lua_tonumber(L,2);
    double r = lua_tonumber(L,3);
    double c = lua_tonumber(L,4);

    _graphicsForLuaApi->circ((short)ox, (short)oy, (short)r, (uint8_t)c);

    return 0;
}
int circfill(lua_State *L){
    double ox = lua_tonumber(L,1);
    double oy = lua_tonumber(L,2);
    double r = lua_tonumber(L,3);
    double c = lua_tonumber(L,4);

    _graphicsForLuaApi->circfill((short)ox, (short)oy, (short)r, (uint8_t)c);

    return 0;
}
int rect(lua_State *L){
    double x1 = lua_tonumber(L,1);
    double y1 = lua_tonumber(L,2);
    double x2 = lua_tonumber(L,3);
    double y2 = lua_tonumber(L,4);
    double c = lua_tonumber(L,5);

    _graphicsForLuaApi->rect((short)x1, (short)y1, (short)x2, (short)y2, (uint8_t)c);

    return 0;
}
int rectfill(lua_State *L){
    double x1 = lua_tonumber(L,1);
    double y1 = lua_tonumber(L,2);
    double x2 = lua_tonumber(L,3);
    double y2 = lua_tonumber(L,4);
    double c = lua_tonumber(L,5);

    _graphicsForLuaApi->rectfill((short)x1, (short)y1, (short)x2, (short)y2, (uint8_t)c);

    return 0;
}
int print(lua_State *L){
    const char * str = lua_tostring(L, 1);
    double x = lua_tonumber(L,2);
    double y = lua_tonumber(L,3);
    double c = lua_tonumber(L,3);

    _graphicsForLuaApi->print(str, x, y, c);

    return 0;
}
int spr(lua_State *L) {
    double n = lua_tonumber(L,1);
    double x = lua_tonumber(L,2);
    double y = lua_tonumber(L,3);
    double w = lua_tonumber(L,4);
    double h = lua_tonumber(L,5);
    bool flip_x = lua_toboolean(L,6);
    bool flip_y = lua_toboolean(L,7);

    _graphicsForLuaApi->spr((short)n, (short)x, (short)y, (short)w, (short)h, flip_x, flip_y);

    return 0;
}

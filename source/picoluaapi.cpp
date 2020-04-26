
#include <string>

#include "picoluaapi.h"
#include "graphics.h"
#include "Input.h"
#include "console.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

Graphics* _graphicsForLuaApi;
Input* _inputForLuaApi;
Console* _consoleForLuaApi;

void initPicoApi(Graphics* graphics, Input* input, Console* console){
    _graphicsForLuaApi = graphics;
    _inputForLuaApi = input;
    _consoleForLuaApi = console;
}

int noop(const char * name) {
    //todo log name of unimplemented functions?
    return 0;
}

int noopreturns(lua_State *L, const char * name) {
    //todo log name of unimplemented functions?
    lua_pushnumber(L, 0);

    return 1;
}

/*functions to expose to lua*/
//Graphics
int cls(lua_State *L){
    if (lua_gettop(L) == 0) {
        _graphicsForLuaApi->cls();
    }
    else {
        short c = lua_tonumber(L,1);
        _graphicsForLuaApi->cls(c);
    }

    return 0;
}

int pset(lua_State *L){
    short x = lua_tonumber(L,1);
    short y = lua_tonumber(L,2);

    if (lua_gettop(L) <= 2) {
        _graphicsForLuaApi->pset(x, y);
        return 0;
    }

    short c = lua_tonumber(L,3);

    _graphicsForLuaApi->pset(x, y, (uint8_t)c);

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
    if (lua_gettop(L) == 0) {
        _graphicsForLuaApi->line();
    }
    else if (lua_gettop(L) == 1) {
        uint8_t c = lua_tonumber(L,1);

        _graphicsForLuaApi->line(c);
    }
    else if (lua_gettop(L) == 2) {
        short x1 = lua_tonumber(L,1);
        short y1 = lua_tonumber(L,2);

        _graphicsForLuaApi->line(x1, y1);
    }
    else if (lua_gettop(L) == 3) {
        short x1 = lua_tonumber(L,1);
        short y1 = lua_tonumber(L,2);
        uint8_t c = lua_tonumber(L,3);

        _graphicsForLuaApi->line(x1, y1, c);
    }
    else if (lua_gettop(L) == 4) {
        short x1 = lua_tonumber(L,1);
        short y1 = lua_tonumber(L,2);
        short x2 = lua_tonumber(L,3);
        short y2 = lua_tonumber(L,4);

        _graphicsForLuaApi->line(x1, y1, x2, y2);
    }
    else {
        short x1 = lua_tonumber(L,1);
        short y1 = lua_tonumber(L,2);
        short x2 = lua_tonumber(L,3);
        short y2 = lua_tonumber(L,4);
        uint8_t c = lua_tonumber(L,5);

        _graphicsForLuaApi->line(x1, y1, x2, y2, c);
    }

    return 0;
}

int circ(lua_State *L){
    short ox = lua_tonumber(L,1);
    short oy = lua_tonumber(L,2);

    if (lua_gettop(L) == 2) {
        _graphicsForLuaApi->circ(ox, oy);
    } 
    else if (lua_gettop(L) == 3){
        short r = lua_tonumber(L,3);
        _graphicsForLuaApi->circ(ox, oy, r);
    }
    else if (lua_gettop(L) > 3){
        short r = lua_tonumber(L,3);
        uint8_t c = lua_tonumber(L,4);

        _graphicsForLuaApi->circ(ox, oy, r, c);
    }

    return 0;
}

int circfill(lua_State *L){
    short ox = lua_tonumber(L,1);
    short oy = lua_tonumber(L,2);

    if (lua_gettop(L) == 2) {
        _graphicsForLuaApi->circfill(ox, oy);
    } 
    else if (lua_gettop(L) == 3){
        short r = lua_tonumber(L,3);
        _graphicsForLuaApi->circfill(ox, oy, r);
    }
    else if (lua_gettop(L) > 3){
        short r = lua_tonumber(L,3);
        uint8_t c = lua_tonumber(L,4);

        _graphicsForLuaApi->circfill(ox, oy, r, c);
    }

    return 0;
}

int rect(lua_State *L){

    if (lua_gettop(L) >= 4) {
        short x1 = lua_tonumber(L,1);
        short y1 = lua_tonumber(L,2);
        short x2 = lua_tonumber(L,3);
        short y2 = lua_tonumber(L,4);

        if (lua_gettop(L) == 4){
            _graphicsForLuaApi->rect(x1, y1, x2, y2);

        }
        else {
            uint8_t c = lua_tonumber(L,5);

            _graphicsForLuaApi->rect(x1, y1, x2, y2, c);
        }
    }

    return 0;
}

int rectfill(lua_State *L){
    if (lua_gettop(L) >= 4) {
        short x1 = lua_tonumber(L,1);
        short y1 = lua_tonumber(L,2);
        short x2 = lua_tonumber(L,3);
        short y2 = lua_tonumber(L,4);

        if (lua_gettop(L) == 4){
            _graphicsForLuaApi->rectfill(x1, y1, x2, y2);

        }
        else {
            double c = lua_tonumber(L,5);

            _graphicsForLuaApi->rectfill(x1, y1, x2, y2, c);
        }
    }

    return 0;
}

int print(lua_State *L){
    if (lua_gettop(L) == 0){
        return 0;
    }

    const char * str = "";

    //todo: handle other cases, maybe move this somewhere else
    //learned this from zepto8 https://github.com/samhocevar/zepto8/blob/27f83fe0626d4823fe2a33568d8310d8def84ae9/src/pico8/vm.cpp
    if (lua_isnil(L, 1)){
        str = "[nil]";
    }
    else if (lua_isstring(L, 1)){
        str = lua_tolstring(L, 1, nullptr);
    }
    else if (lua_isnumber(L, 1)){
        str = lua_tolstring(L, 1, nullptr);
    }
    else if (lua_isboolean(L, 1)){
        str = lua_toboolean(L, 1) ? "true" : "false";
    }

    if (lua_gettop(L) < 3) {
        _graphicsForLuaApi->print(str);
    }
    else if (lua_gettop(L) == 3) {
        short x = lua_tonumber(L,2);
        short y = lua_tonumber(L,3);

        _graphicsForLuaApi->print(str, x, y);
    }
    else {
        short x = lua_tonumber(L,2);
        short y = lua_tonumber(L,3);

        uint8_t c = lua_tonumber(L,4);

        _graphicsForLuaApi->print(str, x, y, c);
    }

    return 0;
}

int spr(lua_State *L) {
    if (lua_gettop(L) < 3) {
        return 0;
    }

    short n = lua_tonumber(L,1);
    short x = lua_tonumber(L,2);
    short y = lua_tonumber(L,3);
    double w = 1.0;
    double h = 1.0;
    bool flip_x = false;
    bool flip_y = false;


    if (lua_gettop(L) > 3){
        w = lua_tonumber(L,4);
    }
    if (lua_gettop(L) > 4) {
        h = lua_tonumber(L,5);
    }
    if (lua_gettop(L) > 5) {
        flip_x = lua_toboolean(L,6);
    }
    if (lua_gettop(L) > 6) {
        flip_y = lua_toboolean(L,7);
    }

    _graphicsForLuaApi->spr(n, x, y, w, h, flip_x, flip_y);

    return 0;
}

int sspr(lua_State *L) {
    if (lua_gettop(L) < 6) {
        return 0;
    }

    short sx = lua_tonumber(L,1);
    short sy = lua_tonumber(L,2);
    short sw = lua_tonumber(L,3);
    short sh = lua_tonumber(L,4);
    short dx = lua_tonumber(L,5);
    short dy = lua_tonumber(L,6);

    short dw = sw;
    short dh = sh;
    bool flip_x = false;
    bool flip_y = false;

    if (lua_gettop(L) > 6){
        dw = lua_tonumber(L,7);
    }
    if (lua_gettop(L) > 7){
        dh = lua_tonumber(L,8);
    }
    if (lua_gettop(L) > 8){
        flip_x = lua_toboolean(L,9);
    }
    if (lua_gettop(L) > 9){
        flip_y = lua_toboolean(L,10);
    }

    _graphicsForLuaApi->sspr(
        sx,
        sy,
        sw,
        sh,
        dx,
        dy,
        dw,
        dh,
        flip_x,
        flip_y);

    return 0;
}

int fget(lua_State *L) {
    double n = lua_tonumber(L,1);

    if (lua_gettop(L) == 1) {
        uint8_t result = _graphicsForLuaApi->fget((uint8_t)n);
        lua_pushinteger(L, result);
    }
    else {
        double f = lua_tonumber(L,2);
        bool result = _graphicsForLuaApi->fget((uint8_t)n, (uint8_t)f);
        lua_pushboolean(L, result);
    }

    return 1;
}

int fset(lua_State *L) {
    double n = lua_tonumber(L,1);

    if (lua_gettop(L) > 2) {
        double f = lua_tonumber(L,2);
        double v = lua_toboolean(L,3);
        _graphicsForLuaApi->fset((uint8_t)n, (uint8_t)f, (bool)v);
    }
    else {
        double v = lua_tonumber(L,2);
        _graphicsForLuaApi->fset((uint8_t)n, (uint8_t)v);
    }

    return 0;
}

int sget(lua_State *L) {
    short x = lua_tonumber(L,1);
    short y = lua_tonumber(L,2);
    uint8_t result = _graphicsForLuaApi->sget((uint8_t)x, (uint8_t)y);
    lua_pushinteger(L, result);

    return 1;
}

int sset(lua_State *L) {
    short x = lua_tonumber(L,1);
    short y = lua_tonumber(L,2);
    uint8_t c = lua_tonumber(L,3);
    _graphicsForLuaApi->sset(x, y, c);

    return 0;
}

int camera(lua_State *L) {
    short x = 0;
    short y = 0;
    if (lua_gettop(L) > 0) {
        x = lua_tonumber(L,1);
    }
    if (lua_gettop(L) > 1) {
        y = lua_tonumber(L,2);
    }
    
    _graphicsForLuaApi->camera(x, y);

    return 0;
}

int clip(lua_State *L) {
    if (lua_gettop(L) >= 4) {
        short x = lua_tonumber(L,1);
        short y = lua_tonumber(L,2);
        short w = lua_tonumber(L,3);
        short h = lua_tonumber(L,4);

        _graphicsForLuaApi->clip(x, y, w, h);
    }
    else {
        _graphicsForLuaApi->clip();
    }

    return 0;
}

int mget(lua_State *L) {
    short celx = lua_tonumber(L,1);
    short cely = lua_tonumber(L,2);

    uint8_t result = _graphicsForLuaApi->mget(celx, cely);
    lua_pushnumber(L, result);

    return 1;
}

int mset(lua_State *L) {
    short celx = lua_tonumber(L,1);
    short cely = lua_tonumber(L,2);
    uint8_t snum = lua_tonumber(L, 3);

    _graphicsForLuaApi->mset(celx, cely, snum);

    return 0;
}

int map(lua_State *L) {
    short celx = lua_tonumber(L,1);
    short cely = lua_tonumber(L,2);
    short sx = lua_tonumber(L,3);
    short sy = lua_tonumber(L,4);
    short celw = lua_tonumber(L,5);
    short celh = lua_tonumber(L,6);
    uint8_t layer = 0;

    if (lua_gettop(L) > 6){
        layer = lua_tonumber(L,7);
    }

    _graphicsForLuaApi->map(celx, cely, sx, sy, celw, celh, layer);

    return 0;
}

int pal(lua_State *L) {
    if (lua_gettop(L) == 0) {
        _graphicsForLuaApi->pal();
        
        return 0;
    }


    uint8_t c0 = lua_tonumber(L,1);
    uint8_t c1 = lua_tonumber(L,2);
    uint8_t p = 0;

    if (lua_gettop(L) > 2){
        p = lua_tonumber(L,3);
    }

    _graphicsForLuaApi->pal(c0, c1, p);

    return 0;
}

int palt(lua_State *L) {
    if (lua_gettop(L) == 0) {
        _graphicsForLuaApi->palt();
        
        return 0;
    }

    uint8_t c = lua_tonumber(L,1);
    bool t = lua_toboolean(L,2);

    _graphicsForLuaApi->palt(c, t);

    return 0;
}

int cursor(lua_State *L) {
    short x = lua_tonumber(L,1);
    short y = lua_tonumber(L,2);

    if (lua_gettop(L) <= 2) {
        _graphicsForLuaApi->cursor(x, y);
        return 0;
    }

    uint8_t c = lua_tonumber(L,3);

    _graphicsForLuaApi->cursor(x, y, c);

    return 0;

}

int fillp(lua_State *L) {
    return noop("fillp");
}

int flip(lua_State *L) {
    return noop("flip");
}

//Input

//input api
int btn(lua_State *L){
    double i = lua_tonumber(L,1);

    bool pressed = _inputForLuaApi->btn((short)i);

    lua_pushboolean(L, pressed);

    return 1;
}
int btnp(lua_State *L){
    double i = lua_tonumber(L,1);

    bool pressed = _inputForLuaApi->btnp((short)i);

    lua_pushboolean(L, pressed);

    return 1;
}

//System
int time(lua_State *L) {
    int frameCount = _consoleForLuaApi->GetFrameCount();
    int targetFps = _consoleForLuaApi->GetTargetFps();

    double seconds = (double)frameCount / (double)targetFps;

    lua_pushnumber(L, seconds);

    return 1;
}

int stat(lua_State *L) {
    return noopreturns(L, "stat");
}

//Audio
int music(lua_State *L) {
    return noop("music");
}

int sfx(lua_State *L) {
    return noop("sfx");
}

//Memory
int cstore(lua_State *L) {
    return noop("cstore");
}

int memcpy(lua_State *L) {
    return noop("memcpy");
}

int memset(lua_State *L) {
    return noop("memset");
}

int peek(lua_State *L) {
    return noopreturns(L, "peek");
}

int poke(lua_State *L) {
    return noop("poke");
}

int reload(lua_State *L) {
    return noop("reload");
}

//cart data
int cartdata(lua_State *L) {
    return noop("cartdata");
}

int dget(lua_State *L) {
    return noopreturns(L, "dget");
}

int dset(lua_State *L) {
    return noop("dset");
}

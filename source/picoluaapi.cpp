
#include <string>
#include <vector>
#include <tuple>
using namespace std;

#include "picoluaapi.h"
#include "graphics.h"
#include "Input.h"
#include "vm.h"
#include "logger.h"
#include "printHelper.h"

//extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
//}

Graphics* _graphicsForLuaApi;
Input* _inputForLuaApi;
Vm* _vmForLuaApi;
Audio* _audioForLuaApi;
PicoRam* _ramForLuaApi;

void initPicoApi(PicoRam* memory, Graphics* graphics, Input* input, Vm* vm, Audio* audio){
    _graphicsForLuaApi = graphics;
    _inputForLuaApi = input;
    _vmForLuaApi = vm;
    _audioForLuaApi = audio;
    _ramForLuaApi = memory;

    initPrintHelper(_ramForLuaApi, _graphicsForLuaApi, _vmForLuaApi, _audioForLuaApi);
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
        int c = lua_tonumber(L,1);
        _graphicsForLuaApi->cls(c);
    }

    return 0;
}

int pset(lua_State *L){
    int x = lua_tonumber(L,1);
    int y = lua_tonumber(L,2);

    if (lua_gettop(L) <= 2) {
        _graphicsForLuaApi->pset(x, y);
        return 0;
    }

    int c = lua_tonumber(L,3);

    _graphicsForLuaApi->pset(x, y, (uint8_t)c);

    return 0;
}

int pget(lua_State *L){
    fix32 x = lua_tonumber(L,1);
    fix32 y = lua_tonumber(L,2);

    uint8_t color = _graphicsForLuaApi->pget((int)x, (int)y);

    lua_pushinteger(L, color);

    return 1;
}

int color(lua_State *L){
    uint8_t prev = 0;
    uint8_t c = 0;
    if (lua_gettop(L) > 0) {
        c = lua_tonumber(L,1);
        prev = _graphicsForLuaApi->color((uint8_t)c);
    }
    else {
        prev = _graphicsForLuaApi->color();
    }

    lua_pushinteger(L, prev);

    return 1;
}

int line (lua_State *L){
    if (lua_gettop(L) == 0) {
        _graphicsForLuaApi->line();
    }
    else if (lua_gettop(L) == 1) {
        fix32 c = lua_tonumber(L,1);

        _graphicsForLuaApi->line(c);
    }
    else if (lua_gettop(L) == 2) {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);

        _graphicsForLuaApi->line(x1, y1);
    }
    else if (lua_gettop(L) == 3) {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        uint8_t c = lua_tonumber(L,3);

        _graphicsForLuaApi->line(x1, y1, c);
    }
    else if (lua_gettop(L) == 4) {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        int x2 = lua_tonumber(L,3);
        int y2 = lua_tonumber(L,4);

        _graphicsForLuaApi->line(x1, y1, x2, y2);
    }
    else {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        int x2 = lua_tonumber(L,3);
        int y2 = lua_tonumber(L,4);
        uint8_t c = lua_tonumber(L,5);

        _graphicsForLuaApi->line(x1, y1, x2, y2, c);
    }

    return 0;
}

int tline (lua_State *L){
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    fix32 mx = 0, my = 0, mdx = fix32::frombits(0x2000), mdy = 0;

    if (lua_gettop(L) >= 6) {
        x0 = lua_tonumber(L,1);
        y0 = lua_tonumber(L,2);
        x1 = lua_tonumber(L,3);
        y1 = lua_tonumber(L,4);
        mx = lua_tonumber(L,5);
        my = lua_tonumber(L,6);
    }
    if (lua_gettop(L) > 7){
        mdx = lua_tonumber(L,7);
        mdy = lua_tonumber(L,8);
    }

    _graphicsForLuaApi->tline(x0, y0, x1, y1, mx, my, mdx, mdy);

    return 0;
}

int circ(lua_State *L){
    int ox = lua_tonumber(L,1);
    int oy = lua_tonumber(L,2);

    if (lua_gettop(L) == 2) {
        _graphicsForLuaApi->circ(ox, oy);
    } 
    else if (lua_gettop(L) == 3){
        int r = lua_tonumber(L,3);
        _graphicsForLuaApi->circ(ox, oy, r);
    }
    else if (lua_gettop(L) > 3){
        int r = lua_tonumber(L,3);
        uint8_t c = lua_tonumber(L,4);

        _graphicsForLuaApi->circ(ox, oy, r, c);
    }

    return 0;
}

int circfill(lua_State *L){
    int ox = lua_tonumber(L,1);
    int oy = lua_tonumber(L,2);

    if (lua_gettop(L) == 2) {
        _graphicsForLuaApi->circfill(ox, oy);
    } 
    else if (lua_gettop(L) == 3){
        int r = lua_tonumber(L,3);
        _graphicsForLuaApi->circfill(ox, oy, r);
    }
    else if (lua_gettop(L) > 3){
        int r = lua_tonumber(L,3);
        uint8_t c = lua_tonumber(L,4);

        _graphicsForLuaApi->circfill(ox, oy, r, c);
    }

    return 0;
}

int oval(lua_State *L){

    if (lua_gettop(L) >= 4) {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        int x2 = lua_tonumber(L,3);
        int y2 = lua_tonumber(L,4);

        if (lua_gettop(L) == 4){
            _graphicsForLuaApi->oval(x1, y1, x2, y2);

        }
        else {
            uint8_t c = lua_tonumber(L,5);

            _graphicsForLuaApi->oval(x1, y1, x2, y2, c);
        }
    }

    return 0;
}

int ovalfill(lua_State *L){
    if (lua_gettop(L) >= 4) {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        int x2 = lua_tonumber(L,3);
        int y2 = lua_tonumber(L,4);

        if (lua_gettop(L) == 4){
            _graphicsForLuaApi->ovalfill(x1, y1, x2, y2);

        }
        else {
            fix32 c = lua_tonumber(L,5);

            _graphicsForLuaApi->ovalfill(x1, y1, x2, y2, c);
        }
    }

    return 0;
}

int rect(lua_State *L){

    if (lua_gettop(L) >= 4) {
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        int x2 = lua_tonumber(L,3);
        int y2 = lua_tonumber(L,4);

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
        int x1 = lua_tonumber(L,1);
        int y1 = lua_tonumber(L,2);
        int x2 = lua_tonumber(L,3);
        int y2 = lua_tonumber(L,4);

        if (lua_gettop(L) == 4){
            _graphicsForLuaApi->rectfill(x1, y1, x2, y2);

        }
        else {
            fix32 c = lua_tonumber(L,5);

            _graphicsForLuaApi->rectfill(x1, y1, x2, y2, c);
        }
    }

    return 0;
}

int print(lua_State *L){
    int numArgs = lua_gettop(L);
    if (numArgs == 0){
        return 0;
    }

    const char * charArray = "";
    size_t len = 0;
    int newx = 0;

    //todo: handle other cases, maybe move this somewhere else
    //learned this from zepto8 https://github.com/samhocevar/zepto8/blob/27f83fe0626d4823fe2a33568d8310d8def84ae9/src/pico8/vm.cpp
    if (lua_isnil(L, 1)){
        charArray = "[nil]";
        len = 5;
    }
    else if (lua_isstring(L, 1)){
        charArray = lua_tolstring(L, 1, &len);
    }
    else if (lua_isnumber(L, 1)){
        charArray = lua_tolstring(L, 1, &len);
    }
    else if (lua_isboolean(L, 1)){
        int boolVal = lua_toboolean(L, 1);
        charArray = boolVal ? "true" : "false";
        len = boolVal ? 4 : 5;
    }
    else if (lua_isfunction(L, 1)){
        charArray = "[function]";
        len = 10;
    }

    std::string str = std::string(charArray, len);

    if (numArgs < 2) {
        newx = print(str);
    }
    else if (numArgs == 2) {
        uint8_t c = lua_tonumber(L,2);

        _graphicsForLuaApi->color(c);
        newx = print(str);
    }
    else if (numArgs == 3) {
        int x = lua_tonumber(L,2);
        int y = lua_tonumber(L,3);

        newx = print(str, x, y);
    }
    else {
        int x = lua_tonumber(L,2);
        int y = lua_tonumber(L,3);

        uint8_t c = lua_tonumber(L,4);

        newx = print(str, x, y, c);
    }

    lua_pushinteger(L, newx);
    return 1;
}

int spr(lua_State *L) {
    if (lua_gettop(L) < 3) {
        return 0;
    }

    int n = lua_tonumber(L,1);
    int x = lua_tonumber(L,2);
    int y = lua_tonumber(L,3);
    fix32 w = 1.0;
    fix32 h = 1.0;
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

    int sx = lua_tonumber(L,1);
    int sy = lua_tonumber(L,2);
    int sw = lua_tonumber(L,3);
    int sh = lua_tonumber(L,4);
    int dx = lua_tonumber(L,5);
    int dy = lua_tonumber(L,6);

    int dw = sw;
    int dh = sh;
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
    fix32 n = lua_tonumber(L,1);

    if (lua_gettop(L) == 1) {
        uint8_t result = _graphicsForLuaApi->fget((uint8_t)n);
        lua_pushinteger(L, result);
    }
    else {
        fix32 f = lua_tonumber(L,2);
        bool result = _graphicsForLuaApi->fget((uint8_t)n, (uint8_t)f);
        lua_pushboolean(L, result);
    }

    return 1;
}

int fset(lua_State *L) {
    fix32 n = lua_tonumber(L,1);

    if (lua_gettop(L) > 2) {
        fix32 f = lua_tonumber(L,2);
        bool v = lua_toboolean(L,3);
        _graphicsForLuaApi->fset((uint8_t)n, (uint8_t)f, v);
    }
    else {
        fix32 v = lua_tonumber(L,2);
        _graphicsForLuaApi->fset((uint8_t)n, (uint8_t)v);
    }

    return 0;
}

int sget(lua_State *L) {
    int x = lua_tonumber(L,1);
    int y = lua_tonumber(L,2);
    uint8_t result = _graphicsForLuaApi->sget((uint8_t)x, (uint8_t)y);
    lua_pushinteger(L, result);

    return 1;
}

int sset(lua_State *L) {
    int x = lua_tonumber(L,1);
    int y = lua_tonumber(L,2);
    uint8_t c = lua_tonumber(L,3);
    _graphicsForLuaApi->sset(x, y, c);

    return 0;
}

int camera(lua_State *L) {
    int16_t x = 0;
    int16_t y = 0;
    if (lua_gettop(L) > 0) {
        x = lua_tointeger(L,1);
    }
    if (lua_gettop(L) > 1) {
        y = lua_tointeger(L,2);
    }
    
    auto prev = _graphicsForLuaApi->camera(x, y);

    lua_pushnumber(L, get<0>(prev));
    lua_pushnumber(L, get<1>(prev));

    return 2;
}

int clip(lua_State *L) {

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> prev;

    if (lua_gettop(L) >= 4) {
        int x = lua_tonumber(L,1);
        int y = lua_tonumber(L,2);
        int w = lua_tonumber(L,3);
        int h = lua_tonumber(L,4);

        prev = _graphicsForLuaApi->clip(x, y, w, h);
    }
    else {
        prev = _graphicsForLuaApi->clip();
    }

    lua_pushnumber(L, get<0>(prev));
    lua_pushnumber(L, get<1>(prev));
    lua_pushnumber(L, get<2>(prev));
    lua_pushnumber(L, get<3>(prev));

    return 4;
}

int mget(lua_State *L) {
    int celx = lua_tonumber(L,1);
    int cely = lua_tonumber(L,2);

    uint8_t result = _graphicsForLuaApi->mget(celx, cely);
    lua_pushnumber(L, result);

    return 1;
}

int mset(lua_State *L) {
    int celx = lua_tonumber(L,1);
    int cely = lua_tonumber(L,2);
    uint8_t snum = lua_tonumber(L, 3);

    _graphicsForLuaApi->mset(celx, cely, snum);

    return 0;
}

int gfx_map(lua_State *L) {
    const bool bigMap = _ramForLuaApi->hwState.mapMemMapping >= 0x80;
	const int bigMapLocation = _ramForLuaApi->hwState.mapMemMapping << 8;
	const int mapSize = bigMap 
		? 0x10000 - bigMapLocation
		: 8192;

	const int mapW = _ramForLuaApi->hwState.widthOfTheMap == 0 ? 256 : _ramForLuaApi->hwState.widthOfTheMap;
	const int mapH = mapSize / mapW;

    int celx = 0, cely = 0, sx = 0, sy = 0, celw = mapW, celh = mapH, argc;
    argc = lua_gettop(L);
    if (argc > 0) {
        celx = lua_tonumber(L,1);
    }
    if (argc > 1) {
        cely = lua_tonumber(L,2);
    }
    if (argc > 2) {
        sx = lua_tonumber(L,3);
    }
    if (argc > 3) {
        sy = lua_tonumber(L,4);
    }
    if (argc > 4) {
        celw = lua_tonumber(L,5);
    }
    if (argc > 5) {
        celh = lua_tonumber(L,6);
    }
    uint8_t layer = 0;

    if (argc > 6){
        layer = lua_tonumber(L,7);
    }

    _graphicsForLuaApi->map(celx, cely, sx, sy, celw, celh, layer);

    return 0;
}

int pal(lua_State *L) {
    int numArgs = lua_gettop(L);
    if (numArgs == 0) {
        _graphicsForLuaApi->pal();

        return 0;
    }

    uint8_t p = 0;
    uint8_t c0 = 0;
    uint8_t c1 = 0;

    if (lua_istable(L, 1)){
        if (numArgs > 1) {
            p = lua_tonumber(L,2);
        }

        /* table is in the stack at index 't' */
        lua_pushnil(L);  /* first key */
        while (lua_next(L, 1) != 0) {
            if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
                c0 = lua_tonumber(L, -2);
                c1 = lua_tonumber(L, -1);

                _graphicsForLuaApi->pal(c0, c1, p);
            }
            lua_pop(L, 1);
        }

        return 0;
    } else if (numArgs == 1) {
        p = lua_tonumber(L, 1);
        
        _graphicsForLuaApi->pal(p);

        return 0;
    }


    c0 = lua_tonumber(L,1);
    c1 = c0;
    if (lua_gettop(L) > 1) {
        c1 = lua_tonumber(L,2);
    }

    if (lua_gettop(L) > 2){
        p = lua_tonumber(L,3);
    }

    uint8_t prev =_graphicsForLuaApi->pal(c0, c1, p);

    lua_pushnumber(L, prev);

    return 1;
}

int palt(lua_State *L) {
    int16_t prev = 0;
    //only 0th color is set to transparent if called with no args
    int16_t c = 0;
    c |= 1UL << 15;
    if (lua_gettop(L) < 2) {
        if (lua_gettop(L) == 1){
            c = lua_tonumber(L,1);
        }
        //c is a bitfield of what colors should be transparent
        for (int i = 0; i < 16; i++){
            //get single bit
            bool bit = (c >> (15 - i)) & 1U;
            auto singlePrev = _graphicsForLuaApi->palt(i, bit);
            //update prev single bit
            if (singlePrev) {
                prev |= 1UL << (15 - i);
            }
        }
    }
    else {
        c = lua_tonumber(L,1);
        bool t = lua_toboolean(L,2);
        prev = _graphicsForLuaApi->palt(c, t);
    }

    lua_pushnumber(L, prev);

    return 1;
}

int cursor(lua_State *L) {
    int x = lua_tonumber(L,1);
    int y = lua_tonumber(L,2);

    std::tuple<uint8_t, uint8_t> prev;

    if (lua_gettop(L) < 3) {
        prev = _graphicsForLuaApi->cursor(x, y);
    }
    else{
        uint8_t c = lua_tonumber(L,3);

        prev =_graphicsForLuaApi->cursor(x, y, c);
    }

    lua_pushnumber(L, get<0>(prev));
    lua_pushnumber(L, get<1>(prev));

    return 2;
}

int fillp(lua_State *L) {
    fix32 pat = 0;
    if (lua_gettop(L) > 0) {
        pat = lua_tonumber(L, 1);
    }

    fix32 prev = _graphicsForLuaApi->fillp(pat);

    lua_pushnumber(L, prev);

    return 1;
}

int flip(lua_State *L) {
    _vmForLuaApi->vm_flip();

    return 0;
}

//Input

//input api
int btn(lua_State *L){
    int numArgs = lua_gettop(L);
    if (numArgs == 0) {
        uint8_t btnstate = _inputForLuaApi->btn();

        lua_pushnumber(L, btnstate);
    }
    else {
        fix32 i = lua_tonumber(L,1);
        int p = 0;
        if (numArgs > 1){
            p = lua_tonumber(L,2);
        };

        bool pressed = _inputForLuaApi->btn((int)i, p);

        lua_pushboolean(L, pressed);
    }

    return 1;
}
int btnp(lua_State *L){
    int numArgs = lua_gettop(L);
    if (numArgs == 0) {
        uint8_t btnpstate = _inputForLuaApi->btnp();

        lua_pushnumber(L, btnpstate);
    }
    else {
        fix32 i = lua_tonumber(L,1);
        int p = 0;
        if (numArgs > 1){
            p = lua_tonumber(L,2);
        };

        bool pressed = _inputForLuaApi->btnp((int)i, p);

        lua_pushboolean(L, pressed);
    }

    return 1;
}

//System
int time(lua_State *L) {
    int frameCount = _vmForLuaApi->GetFrameCount();
    int targetFps = _vmForLuaApi->GetTargetFps();

    fix32 seconds = (fix32)frameCount / (fix32)targetFps;

    lua_pushnumber(L, seconds);

    return 1;
}

int stat(lua_State *L) {
    int n = (int)lua_tonumber(L, 1);

    switch(n){
        //0 memory usage
        case 0:
            //TODO: get from z8lua
            lua_pushnumber(L, 1);
            return 1;
        break;
        //cpu usage
        case 1:
            // stubbed to return 50%
            lua_pushnumber(L, 0.5);
            return 1;
        break;
        //cpu usage (without system calls)
        case 2:
            // stubbed to return 50%
            lua_pushnumber(L, 0.5);
            return 1;
        break;
        //clipboard contents
        case 4:
            // no clipboard support currently
            lua_pushstring(L, "");
            return 1;
        break;
        //version
        case 5:
            // no clipboard support currently
            lua_pushnumber(L, 22);
            return 1;
        break;
        //argument
        case 6:
            // no args or loading other cards currently supported
            lua_pushstring(L, _vmForLuaApi->getCartParam().c_str());
            return 1;
        break;
        //frame rate
        case 7:
            lua_pushnumber(L, _vmForLuaApi->getFps());
            return 1;
        break;
        //target framerate
        case 8:
            lua_pushnumber(L, _vmForLuaApi->getTargetFps());
            return 1;
        break;
        //16-19 audio sfx currently playing
        case 16:
        case 46:
            lua_pushnumber(L, _audioForLuaApi->getCurrentSfxId(0));
            return 1;
        break;
        case 17:
        case 47:
            lua_pushnumber(L, _audioForLuaApi->getCurrentSfxId(1));
            return 1;
        break;
        case 18:
        case 48:
            lua_pushnumber(L, _audioForLuaApi->getCurrentSfxId(2));
            return 1;
        break;
        case 19:
        case 49:
            lua_pushnumber(L, _audioForLuaApi->getCurrentSfxId(3));
            return 1;
        break;
        //20-23 note idx of sfx currently playing
        case 20:
        case 50:
            lua_pushnumber(L, _audioForLuaApi->getCurrentNoteNumber(0));
            return 1;
        break;
        case 21:
        case 51:
            lua_pushnumber(L, _audioForLuaApi->getCurrentNoteNumber(1));
            return 1;
        break;
        case 22:
        case 52:
            lua_pushnumber(L, _audioForLuaApi->getCurrentNoteNumber(2));
            return 1;
        break;
        case 23:
        case 53:
            lua_pushnumber(L, _audioForLuaApi->getCurrentNoteNumber(3));
            return 1;
        break;
        //current music pattern
        case 24:
        case 54:
            lua_pushnumber(L, _audioForLuaApi->getCurrentMusic());
            return 1;
        break;
        //current music count
        case 25:
        case 55:
            lua_pushnumber(L, _audioForLuaApi->getMusicPatternCount());
            return 1;
        break;
        //current music tick count
        case 56:
        case 26:
            lua_pushnumber(L, _audioForLuaApi->getMusicTickCount());
            return 1;
        break;
        //if SDL scancode is pressed. always false for now
        case 28:
            lua_pushboolean(L, false);
            return 1;
        break;
        //unknown. appears to always be 0
        case 29:
            lua_pushnumber(L, 0);
            return 1;
        break;
        //was a key pressed 
        case 30:
            lua_pushboolean(L, _inputForLuaApi->getKeyDown());			
            return 1;
        break;
        //string of key pressed
        case 31:
            lua_pushstring(L, _inputForLuaApi->getKey());
            return 1;
        break;
        //mouse x
        case 32:
            lua_pushnumber(L, _inputForLuaApi->getMouseX());
            return 1;
        break;
        //mouse y
        case 33:
            lua_pushnumber(L, _inputForLuaApi->getMouseY());
            return 1;
        break;
        //mouse btn state
        case 34:
            lua_pushnumber(L, _inputForLuaApi->getMouseBtnState());
            return 1;
        break;
        //Current Year
        case 90:
            lua_pushnumber(L, _vmForLuaApi->getYear());
            return 1;
        break;
        //Current month
        case 91:
            lua_pushnumber(L, _vmForLuaApi->getMonth());
            return 1;
        break;
        //Current day
        case 92:
            lua_pushnumber(L, _vmForLuaApi->getDay());
            return 1;
        break;
        //Current Hour
        case 93:
            lua_pushnumber(L, _vmForLuaApi->getHour());
            return 1;
        break;
        //Current Minute
        case 94:
            lua_pushnumber(L, _vmForLuaApi->getMinute());
            return 1;
        break;
        //Current second
        case 95:
            lua_pushnumber(L, _vmForLuaApi->getSecond());
            return 1;
        break;
        case 100:
            lua_pushstring(L, _vmForLuaApi->getCartBreadcrumb().c_str());
            return 1;
        //unknown? used by serial carts
        case 108:
            lua_pushnumber(L, 32);
            return 1;
        break;
        //bytestream availability (dropped file or image. not supported)
        case 120:
            lua_pushboolean(L, false);
            return 1;
        break;
        case 121:
            lua_pushboolean(L, false);
            return 1;
        break;
        case 122:
            lua_pushboolean(L, false);
            return 1;
        break;
    }


    return noopreturns(L, "stat");
}

//Audio
int music(lua_State *L) {
    int n = lua_tonumber(L,1);
    int fadems = 0;
    if (lua_gettop(L) > 1) {
        fadems = (int)lua_tonumber(L, 2);
    }
    int channelmask = 0;
    if (lua_gettop(L) > 2) {
        channelmask = (int)lua_tonumber(L, 3);
    }

    _audioForLuaApi->api_music(n, fadems, channelmask);
    return 0;
}

int sfx(lua_State *L) {
    fix32 n = lua_tonumber(L,1);
    int channel = -1;
    if (lua_gettop(L) > 1) {
        channel = (int)lua_tonumber(L, 2);
    }
    int offset = 0;
    if (lua_gettop(L) > 2) {
        offset = (int)lua_tonumber(L, 3);
    }

    _audioForLuaApi->api_sfx((int)n, channel, offset);
    return 0;
}

//Memory
int cstore(lua_State *L) {
    //this is supposed to copy data from ram to the file.
    //for now, not implementing this
    return noop("cstore");
}

int api_memcpy(lua_State *L) {
    uint16_t dest = (uint16_t)lua_tointeger(L,1);
    uint16_t src = (uint16_t)lua_tointeger(L,2);
    uint16_t len = (uint16_t)lua_tointeger(L,3);

    _vmForLuaApi->vm_memcpy(dest, src, len);

    return 0;
}

int api_memset(lua_State *L) {
    uint16_t dest = (uint16_t)lua_tointeger(L,1);
    uint16_t val = (uint16_t)lua_tointeger(L,2);
    uint16_t len = (uint16_t)lua_tointeger(L,3);

    _vmForLuaApi->vm_memset(dest, val, len);

    return 0;
}

int peek(lua_State *L) {
    int numArgs = lua_gettop(L);
    int numToReturn = 1;

    uint16_t addr = (uint16_t)lua_tointeger(L,1);

    if (numArgs > 1) {
        numToReturn = lua_tonumber(L,2);
    }

    for(int i = 0; i < numToReturn; i++) {
        uint8_t val = _vmForLuaApi->vm_peek(addr + i);

        lua_pushinteger(L, val);
    }

    return numToReturn;
}

int poke(lua_State *L) {
    int numArgs = lua_gettop(L);

    uint16_t dest = (uint16_t)lua_tointeger(L,1);
    uint8_t val = 0;
    if (numArgs > 1) {
        val = lua_tonumber(L,2);
    }

    _vmForLuaApi->vm_poke(dest, val);

    if (numArgs > 2) {
        for(int i = 1; i <= (numArgs - 2); i++) {
            val = lua_tonumber(L, 2 + i);
            _vmForLuaApi->vm_poke(dest + i, val);
        }
    }

    return 0;
}

int peek2(lua_State *L) {
    uint16_t addr = (uint16_t)lua_tointeger(L,1);

    int16_t val = _vmForLuaApi->vm_peek2(addr);

    lua_pushinteger(L, val);

    return 1;
}

int poke2(lua_State *L) {
    int numArgs = lua_gettop(L);

    uint16_t dest = (uint16_t)lua_tointeger(L,1);

    int val = 0;
    if (numArgs > 1) {
        val = lua_tonumber(L,2);
    }

    _vmForLuaApi->vm_poke2(dest, (int16_t)val);

    if (numArgs > 2) {
        for(int i = 1; i <= (numArgs - 2); i++) {
            val = lua_tonumber(L, 2 + i);
            _vmForLuaApi->vm_poke2(dest + i, (int16_t)val);
        }
    }

    return 0;
}

int peek4(lua_State *L) {
    uint16_t addr = (uint16_t)lua_tointeger(L,1);

    fix32 val = _vmForLuaApi->vm_peek4(addr);

    lua_pushnumber(L, val);

    return 1;
}

int poke4(lua_State *L) {
    int numArgs = lua_gettop(L);

    uint16_t dest = (uint16_t)lua_tointeger(L,1);

    fix32 val = 0;
    if (numArgs > 1) {
        val = lua_tonumber(L,2);
    }

    _vmForLuaApi->vm_poke4(dest, val);

    if (numArgs > 2) {
        for(int i = 1; i <= (numArgs - 2); i++) {
            val = lua_tonumber(L, 2 + i);
            _vmForLuaApi->vm_poke4(dest + i, val);
        }
    }

    return 0;
}

int reload(lua_State *L) {
    uint16_t dest = 0;
    uint16_t src = 0;
    uint16_t len = 0x4300;
    const char * str = "";
    if (lua_gettop(L) > 0) {
        dest = (uint16_t)lua_tointeger(L,1);
    }
    if (lua_gettop(L) > 1) {
        src = (uint16_t)lua_tointeger(L,2);
    }
    if (lua_gettop(L) > 2) {
        len = (uint16_t)lua_tointeger(L,3);
    }
    if (lua_gettop(L) > 3) {
        str = lua_tolstring(L, 4, nullptr);
        if (str == nullptr) {
            str = "";
        }
    }

    _vmForLuaApi->vm_reload(dest, src, len, str);

    return 0;
}

//cart data
int cartdata(lua_State *L) {
    bool result = false;

    if (lua_gettop(L) > 0) {
        std::string key = lua_tolstring(L, 1, nullptr);
        result = _vmForLuaApi->vm_cartdata(key);
    }

    lua_pushboolean(L, result);

    return 1;
}

int dget(lua_State *L) {
    int addr = lua_tonumber(L,1);

    fix32 val = _vmForLuaApi->vm_dget(addr);

    lua_pushnumber(L, val);

    return 1;
}

int dset(lua_State *L) {
    int dest = lua_tonumber(L,1);
    fix32 val = lua_tonumber(L,2);

    _vmForLuaApi->vm_dset(dest, val);

    return 0;
}

int printh(lua_State *L) {
    if (lua_isstring(L, 1)){
        const char * str = "";
        str = lua_tolstring(L, 1, nullptr);
        printf("%s\n", str);
    }
    return 0;
}

int rnd(lua_State *L) {
    if (lua_gettop(L) == 0) {
        fix32 val = _vmForLuaApi->api_rnd();

        lua_pushnumber(L, val);
    }
    else {
        if (lua_istable(L, 1)){
            size_t len = lua_rawlen(L,1);
            fix32 range = (fix32)len;
            int idx = (int)(_vmForLuaApi->api_rnd(range)) + 1;
            
            lua_rawgeti(L, 1, idx);
        }
        else {
            fix32 range = lua_tonumber(L,1);
            fix32 val = _vmForLuaApi->api_rnd(range);

            lua_pushnumber(L, val);
        }
    }

    return 1;
}

int srand(lua_State *L) {
    fix32 seed = lua_tonumber(L,1);
    _vmForLuaApi->api_srand(seed);

    return 0;
}

int _update_buttons(lua_State *L) {
    _vmForLuaApi->update_buttons();
    
    return 0;
}

int run(lua_State *L) {
    _vmForLuaApi->vm_run();
    
    return 0;
}

int extcmd(lua_State *L){
    const char * str = "";

    if (lua_isstring(L, 1)){
        str = lua_tolstring(L, 1, nullptr);
    }

    _vmForLuaApi->vm_extcmd(str);

    return 0;
}

int load(lua_State *L) {
    const char* filename = "";
    const char* breadcrumb = "";
    const char* param = "";
    int argC = lua_gettop(L);
    if (argC > 0 && lua_isstring(L, 1)){
        filename = lua_tolstring(L, 1, nullptr);
        if (argC > 1 && lua_isstring(L, 2)){
            breadcrumb = lua_tolstring(L, 2, nullptr);
        }
        if (argC > 2 && lua_isstring(L, 3)){
            param = lua_tolstring(L, 3, nullptr);
        }

        _vmForLuaApi->vm_load(filename, breadcrumb, param);
    }

    return 0;
}

int reset(lua_State *L) {
    _vmForLuaApi->vm_reset();

    return 0;
}

int setFps(lua_State *L){
    _vmForLuaApi->setTargetFps(lua_tointeger(L, 1));

    return 0;
}

int listcarts(lua_State *L) {
    //get cart list from VM (who should get it from host)
    vector<string> carts = _vmForLuaApi->GetCartList();

    lua_createtable(L, carts.size(), 0);
    int newTable = lua_gettop(L);
    int index = 1;

    for(size_t i = 0; i < carts.size(); i++){
        lua_pushstring(L, carts[i].c_str());
        lua_rawseti(L, newTable, index);
        
        ++index;
    }

    return 1;
}



int getbioserror(lua_State *L) {
    string error = _vmForLuaApi->GetBiosError();

    lua_pushstring(L, error.c_str());

    return 1;
}

int loadbioscart(lua_State *L) {
    _vmForLuaApi->QueueCartChange("__FAKE08-BIOS.p8");

    return 0;
}

int loadsettingscart(lua_State *L) {
    _vmForLuaApi->QueueCartChange("__FAKE08-SETTINGS.p8");

    return 0;
}

int togglepausemenu(lua_State *L) {
    _vmForLuaApi->togglePauseMenu();

    return 0;
}

int resetcart(lua_State *L) {
    _vmForLuaApi->QueueCartChange(_vmForLuaApi->CurrentCartFilename());

    return 0;
}


int getsetting(lua_State *L) {
    //get setting from host
	
	const char * str = "";
	if (lua_isstring(L, 1)){
        str = lua_tolstring(L, 1, nullptr);
    }
	Logger_Write("loading setting ");
	Logger_Write(str);
	Logger_Write("\n");
	//std::string sname = str;
	
	int val = _vmForLuaApi->getSetting(str);
	
	lua_pushnumber(L, val);

    return 1;
}

int setsetting(lua_State *L) {
    //get setting from host
	const char * str = "";
	if (lua_isstring(L, 1)){
        str = lua_tolstring(L, 1, nullptr);
    }
	Logger_Write("setting setting ");
	Logger_Write(str);
	Logger_Write("\n");
	
	int sval = lua_tonumber(L,2);
	
	_vmForLuaApi->setSetting(str,sval);
	
    return 1;
}



int installpackins(lua_State *L) {
    #if LOAD_PACK_INS
	_vmForLuaApi->installPackins();
	#endif
    return 1;
}


int loadlabel(lua_State *L) {
	
	const char * cartname = "";
	if (lua_isstring(L, 1)){
        cartname = lua_tolstring(L, 1, nullptr);
    }
	std::string filename = cartname;
	
	bool mini = lua_toboolean(L,2);
	int minioffset = lua_tonumber(L,3);
	
	_vmForLuaApi->loadLabel(filename, mini, minioffset);
	return 1;
}


int getlualine(lua_State *L) {	
	const char * cartname = "";
	if (lua_isstring(L, 1)){
        cartname = lua_tolstring(L, 1, nullptr);
    }
	std::string filename = cartname;
	
	int linenumber = lua_tonumber(L,2);
	
	std::string resultstring = _vmForLuaApi->getLuaLine(filename, linenumber);
	
	lua_pushstring(L, resultstring.c_str());
	
	return 1;
}

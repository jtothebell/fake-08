
#include <string>
#include <vector>
using namespace std;

#include "picoluaapi.h"
#include "graphics.h"
#include "Input.h"
#include "vm.h"
#include "logger.h"

//extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
//}

Graphics* _graphicsForLuaApi;
Input* _inputForLuaApi;
Vm* _vmForLuaApi;
Audio* _audioForLuaApi;

void initPicoApi(Graphics* graphics, Input* input, Vm* vm, Audio* audio){
    _graphicsForLuaApi = graphics;
    _inputForLuaApi = input;
    _vmForLuaApi = vm;
    _audioForLuaApi = audio;
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
    double x = lua_tonumber(L,1);
    double y = lua_tonumber(L,2);

    uint8_t color = _graphicsForLuaApi->pget((int)x, (int)y);

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
        int x = lua_tonumber(L,2);
        int y = lua_tonumber(L,3);

        _graphicsForLuaApi->print(str, x, y);
    }
    else {
        int x = lua_tonumber(L,2);
        int y = lua_tonumber(L,3);

        uint8_t c = lua_tonumber(L,4);

        _graphicsForLuaApi->print(str, x, y, c);
    }

    return 0;
}

int spr(lua_State *L) {
    if (lua_gettop(L) < 3) {
        return 0;
    }

    int n = lua_tonumber(L,1);
    int x = lua_tonumber(L,2);
    int y = lua_tonumber(L,3);
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
    int x = 0;
    int y = 0;
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
        int x = lua_tonumber(L,1);
        int y = lua_tonumber(L,2);
        int w = lua_tonumber(L,3);
        int h = lua_tonumber(L,4);

        _graphicsForLuaApi->clip(x, y, w, h);
    }
    else {
        _graphicsForLuaApi->clip();
    }

    return 0;
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

int map(lua_State *L) {
    int celx = lua_tonumber(L,1);
    int cely = lua_tonumber(L,2);
    int sx = lua_tonumber(L,3);
    int sy = lua_tonumber(L,4);
    int celw = lua_tonumber(L,5);
    int celh = lua_tonumber(L,6);
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
    int x = lua_tonumber(L,1);
    int y = lua_tonumber(L,2);

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

    bool pressed = _inputForLuaApi->btn((int)i);

    lua_pushboolean(L, pressed);

    return 1;
}
int btnp(lua_State *L){
    double i = lua_tonumber(L,1);

    bool pressed = _inputForLuaApi->btnp((int)i);

    lua_pushboolean(L, pressed);

    return 1;
}

//System
int time(lua_State *L) {
    int frameCount = _vmForLuaApi->GetFrameCount();
    int targetFps = _vmForLuaApi->GetTargetFps();

    double seconds = (double)frameCount / (double)targetFps;

    lua_pushnumber(L, seconds);

    return 1;
}

int stat(lua_State *L) {
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
    double n = lua_tonumber(L,1);
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
    int dest = lua_tonumber(L,1);
    int src = lua_tonumber(L,2);
    int len = lua_tonumber(L,3);

    _vmForLuaApi->vm_memcpy(dest, src, len);

    return 0;
}

int api_memset(lua_State *L) {
    int dest = lua_tonumber(L,1);
    int val = lua_tonumber(L,2);
    int len = lua_tonumber(L,3);

    _vmForLuaApi->vm_memset(dest, val, len);

    return 0;
}

int peek(lua_State *L) {
    int addr = lua_tonumber(L,1);

    uint8_t val = _vmForLuaApi->vm_peek(addr);

    lua_pushinteger(L, val);

    return 1;
}

int poke(lua_State *L) {
    int dest = lua_tonumber(L,1);
    int val = lua_tonumber(L,2);

    _vmForLuaApi->vm_poke(dest, (uint8_t)val);

    return 0;
}

int peek2(lua_State *L) {
    int addr = lua_tonumber(L,1);

    int16_t val = _vmForLuaApi->vm_peek2(addr);

    lua_pushinteger(L, val);

    return 1;
}

int poke2(lua_State *L) {
    int dest = lua_tonumber(L,1);
    int val = lua_tonumber(L,2);

    _vmForLuaApi->vm_poke2(dest, (int16_t)val);

    return 0;
}

int peek4(lua_State *L) {
    int addr = lua_tonumber(L,1);

    int32_t val = _vmForLuaApi->vm_peek4(addr);

    lua_pushinteger(L, val);

    return 1;
}

int poke4(lua_State *L) {
    int dest = lua_tonumber(L,1);
    int val = lua_tonumber(L,2);

    _vmForLuaApi->vm_poke4(dest, (int32_t)val);

    return 0;
}

int reload(lua_State *L) {
    int dest = lua_tonumber(L,1);
    int src = lua_tonumber(L,2);
    int len = lua_tonumber(L,3);
    const char * str = "";

    if (lua_gettop(L) > 3) {
        str = lua_tolstring(L, 4, nullptr);
    }

    _vmForLuaApi->vm_reload(dest, src, len, str);

    return 0;
}

//cart data
int cartdata(lua_State *L) {
    const char * str = lua_tolstring(L, 1, nullptr);

    _vmForLuaApi->vm_cartdata(str);

    return 0;
}

int dget(lua_State *L) {
    int addr = lua_tonumber(L,1);

    int32_t val = _vmForLuaApi->vm_dget(addr);

    lua_pushinteger(L, val);

    return 1;
}

int dset(lua_State *L) {
    int dest = lua_tonumber(L,1);
    int val = lua_tonumber(L,2);

    _vmForLuaApi->vm_dset(dest, (int32_t)val);

    return 0;
}

int printh(lua_State *L) {
    if (lua_isstring(L, 1)){
        const char * str = "";
        str = lua_tolstring(L, 1, nullptr);
        printf("%s", str);
    }
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

int loadcart(lua_State *L) {
    if (lua_isstring(L, 1)){
        const char * str = "";
        str = lua_tolstring(L, 1, nullptr);
        _vmForLuaApi->QueueCartChange(str);
    }

    return 0;
}

int getbioserror(lua_State *L) {
    string error = _vmForLuaApi->GetBiosError();

    lua_pushstring(L, error.c_str());

    return 1;
}

int loadbioscart(lua_State *L) {
    //_vmForLuaApi->LoadBiosCart();

    return 0;
}

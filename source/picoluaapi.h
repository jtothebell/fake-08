#pragma once

#include <string>

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#include "graphics.h"
#include "Input.h"

//this can probably go away when I'm loading actual carts and just have to expose api to lua
void initPicoApi(Graphics* graphics, Input* input);

//graphics api
int cls(lua_State *L);
int pset(lua_State *L);
int pget(lua_State *L);
int color(lua_State *L);
int line (lua_State *L);
int circ(lua_State *L);
int circfill(lua_State *L);
int rect(lua_State *L);
int rectfill(lua_State *L);
int print(lua_State *L);
int spr(lua_State *L);
int sspr(lua_State *L);
int fget(lua_State *L);
int fset(lua_State *L);

//input api
int btn(lua_State *L);
int btnp(lua_State *L);


//TODO:
//graphics

//api.fget(n, f)

//api.fset(n, f, v)

//api.sget(x, y)

//api.sset(x, y, c)

//api.flip()
//int flip(lua_State *L);
//api.camera(x, y)
//int camera(lua_State *L);
//api.clip(x, y, w, h)
//int clip(lua_State *L);
//api.cursor(x, y)
//int cursor(lua_State *L);
//api.pal(c0, c1, p)

//api.palt(c, t)

//api.fillp(p)

//map
//api.map(cel_x, cel_y, sx, sy, cel_w, cel_h, bitmask)

//api.mget(x, y)

//api.mset(x, y, v)


//system functions
//api.time() and api.t()

//api.tonum(val)

//api.tostr(val, hex)

//api.rnd(x)

//api.srand(seed)

//api.flr=math.floor

//api.ceil=math.ceil

//api.sgn(x)

//api.abs=math.abs

//api.min(a, b)

//api.max(a, b)

//api.mid(x, y, z)

//api.cos(x)

//api.sin(x)

//api.sqrt=math.sqrt

//api.atan2(x, y)

//api.band(x, y)

//api.bor(x, y)

//api.bxor(x, y)

//api.bnot(x)

//api.shl(x, y)

//api.shr(x, y)

//api.lshr(x, y)

//api.rotl(x, y)

//api.rotr(x, y)

//sound
//api.music(n, fade_len, channel_mask)

//api.sfx(n, channel, offset)


//not to do any time soon, but don't crash?
//directory stuff
//api.load(filename)

//api.save()

//api.run()

//api.stop()

//api.reboot()

//api.shutdown()

//api.exit()

//api.info()

//api.export()

//api.import()

//api.help()

//api.folder()

//api.ls()

//api.dir=api.ls

//api.cd()

//api.mkdir()

//api.install_demos()

//api.install_games()

//api.keyconfig()

//api.splore()

//memory
//api.peek(addr)

//api.poke(addr, val)

//api.peek4(addr)

//api.poke4(addr, val)

//api.memcpy(dest_addr, source_addr, len)

//api.memset(dest_addr, val, len)

//api.reload(dest_addr, source_addr, len)

//api.cstore(dest_addr, source_addr, len)

//
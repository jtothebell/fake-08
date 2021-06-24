#pragma once

#include <string>

//extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
//}

#include "graphics.h"
#include "Input.h"
#include "vm.h"

//this can probably go away when I'm loading actual carts and just have to expose api to lua
void initPicoApi(Graphics* graphics, Input* input, Vm* vm, Audio* audio);

//graphics api
int cls(lua_State *L);
int pset(lua_State *L);
int pget(lua_State *L);
int color(lua_State *L);
int line (lua_State *L);
int tline (lua_State *L);
int circ(lua_State *L);
int circfill(lua_State *L);
int oval(lua_State *L);
int ovalfill(lua_State *L);
int rect(lua_State *L);
int rectfill(lua_State *L);
int print(lua_State *L);
int spr(lua_State *L);
int sspr(lua_State *L);
int fget(lua_State *L);
int fset(lua_State *L);
int sget(lua_State *L);
int sset(lua_State *L);
int camera(lua_State *L);
int clip(lua_State *L);
int mget(lua_State *L);
int mset(lua_State *L);
int gfx_map(lua_State *L);
int pal(lua_State *L);
int palt(lua_State *L);
int cursor(lua_State *L);
//todo:
int fillp(lua_State *L); 
int flip(lua_State *L);
//end todo

//tables functions, maybe implemented in lua? 
//list from: https://pico-8.fandom.com/wiki/APIReference
/*
     add(t, v)
    all(t)
    del(t, v)
    foreach(t, f)
    pairs(t) 
*/

//input api
int btn(lua_State *L);
int btnp(lua_State *L);

//audio api
int music(lua_State *L);
int sfx(lua_State *L);

//system
int time(lua_State *L);
int stat(lua_State *L);

//memory api
int cstore(lua_State *L);
int api_memcpy(lua_State *L);
int api_memset(lua_State *L);
int peek(lua_State *L);
int poke(lua_State *L);
int peek2(lua_State *L);
int poke2(lua_State *L);
int peek4(lua_State *L);
int poke4(lua_State *L);
int reload(lua_State *L);

//cart data
int cartdata(lua_State *L);
int dget(lua_State *L);
int dset(lua_State *L);

//
int printh(lua_State *L);

//file system/vm functions
int listcarts(lua_State *L);

int getbioserror(lua_State *L);
int loadbioscart(lua_State *L);
int togglepausemenu(lua_State *L);
int resetcart(lua_State *L);

//system functions

int rnd(lua_State *L);
int srand(lua_State *L);
int _update_buttons(lua_State *L);
int run(lua_State *L);
int extcmd(lua_State *L);
int load(lua_State *L);

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
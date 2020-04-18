#pragma once

#include <string>

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

#include "graphics.h"

//this can probably go away when I'm loading actual carts and just have to expose api to lua
void initPicoApi(Graphics* graphics);

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


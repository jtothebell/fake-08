#pragma once

#include "cart.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

class Console {
    Cart* _loadedCart;
    Graphics* _graphics;
    lua_State* _luaState;

    public:
    Console();

    void LoadCart(std::string filename);

    void UpdateAndDraw(int frameCount);

    void FlipBuffer(uint8_t* fb);

    void TurnOff();
};


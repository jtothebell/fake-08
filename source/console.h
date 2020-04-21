#pragma once

#include "cart.h"
#include "Input.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

class Console {
    Cart* _loadedCart;
    Graphics* _graphics;
    lua_State* _luaState;
    Input* _input;

    public:
    Console();
    ~Console();

    void LoadCart(std::string filename);

    void UpdateAndDraw(int frameCount, uint8_t kdown, uint8_t kheld);

    void FlipBuffer(uint8_t* fb);

    void TurnOff();
};


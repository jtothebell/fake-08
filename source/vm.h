#pragma once

#include "cart.h"
#include "Input.h"
#include "Audio.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

class Vm {
    Cart* _loadedCart;
    Graphics* _graphics;
    Audio* _audio;
    lua_State* _luaState;
    Input* _input;

    uint8_t _targetFps;

    int _picoFrameCount;
    bool _hasUpdate;
    bool _hasDraw;

    public:
    Vm();
    ~Vm();

    void LoadCart(std::string filename);

    void UpdateAndDraw(
      uint8_t kdown,
      uint8_t kheld);

    uint8_t* GetPicoInteralFb();
    uint8_t* GetScreenPaletteMap();
    Color* GetPaletteColors();

    void FillAudioBuffer(void *audioBuffer, size_t offset, size_t size);

    void TurnOff();

    uint8_t GetTargetFps();

    int GetFrameCount();
};


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

    int _targetFps;

    int _picoFrameCount;
    bool _hasUpdate;
    bool _hasDraw;

    bool _cartChangeQueued;
    std::string _nextCartKey;

    bool loadCart(Cart* cart);

    public:
    Vm();
    ~Vm();

    void LoadBiosCart();

    void LoadCart(std::string filename);

    void UpdateAndDraw(
      uint8_t kdown,
      uint8_t kheld);

    uint8_t* GetPicoInteralFb();
    uint8_t* GetScreenPaletteMap();
    Color* GetPaletteColors();

    void FillAudioBuffer(void *audioBuffer, size_t offset, size_t size);

    void CloseCart();

    void QueueCartChange(std::string newcart);

    int GetTargetFps();

    int GetFrameCount();
};


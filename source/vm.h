#pragma once

#include <vector>
#include <string>
using namespace std;

#include "cart.h"
#include "Input.h"
#include "Audio.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

class Vm {
    PicoRam _memory;

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
    string _nextCartKey;

    string _cartLoadError;

    vector<string> _cartList;

    bool loadCart(Cart* cart);

    public:
    Vm();
    ~Vm();

    void LoadBiosCart();

    void LoadCart(string filename);

    void UpdateAndDraw(
      uint8_t kdown,
      uint8_t kheld);

    uint8_t* GetPicoInteralFb();
    uint8_t* GetScreenPaletteMap();
    Color* GetPaletteColors();

    void FillAudioBuffer(void *audioBuffer, size_t offset, size_t size);

    void CloseCart();

    void QueueCartChange(string newcart);

    int GetTargetFps();

    int GetFrameCount();

    void SetCartList(vector<string> cartList);
    vector<string> GetCartList();
    string GetBiosError();

    bool ExecuteLua(string luaString, string callbackFunction);

    PicoRam* getPicoRam();

    uint8_t ram_peek(int addr);
    int16_t ram_peek2(int addr);
    int32_t ram_peek4(int addr); //note: this should return a 32 bit fixed point number

    void ram_poke(int addr, uint8_t value);
    void ram_poke2(int addr, int16_t value);
    void ram_poke4(int addr, int32_t value); //note: this parameter should be a 32 bit fixed point number
};


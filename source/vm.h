#pragma once

#include <vector>
#include <string>
using namespace std;

#include "cart.h"
#include "Input.h"
#include "Audio.h"

//extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
//}

class Vm {
    PicoRam* _memory;

    Graphics* _graphics;
    Audio* _audio;
    Input* _input;

    Cart* _loadedCart;
    lua_State* _luaState;

    bool _cleanupDeps;

    int _targetFps;

    int _picoFrameCount;
    bool _hasUpdate;
    bool _hasDraw;

    bool _cartChangeQueued;
    string _nextCartKey;

    string _cartLoadError;

    string _cartdataKey;

    vector<string> _cartList;

    bool loadCart(Cart* cart);
    void vm_reload(int destaddr, int sourceaddr, int len, Cart* cart);

    public:
    Vm(PicoRam* memory = nullptr,
       Graphics* graphics = nullptr,
       Input* input = nullptr,
       Audio* audio = nullptr);
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

    uint8_t vm_peek(int addr);
    int16_t vm_peek2(int addr);
    int32_t vm_peek4(int addr); //note: this should return a 32 bit fixed point number

    void vm_poke(int addr, uint8_t value);
    void vm_poke2(int addr, int16_t value);
    void vm_poke4(int addr, int32_t value); //note: this parameter should be a 32 bit fixed point number

    void vm_cartdata(string key);
    int32_t vm_dget(uint8_t n);
    void vm_dset(uint8_t n, int32_t value); //note: this should return a 32 bit fixed point number

    void vm_reload(int destaddr, int sourceaddr, int len, string filename);

    void vm_memset(int destaddr, uint8_t val, int len);
    void vm_memcpy(int destaddr, int sourceaddr, int len);
};


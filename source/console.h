#pragma once
#include <functional>

#include "cart.h"
#include "Input.h"
#include "Audio.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

enum StretchOption {
  PixelPerfect,
  StretchToFit,
  StretchAndOverflow
};

class Console {
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
    Console();
    ~Console();

    void LoadCart(std::string filename);

    void UpdateAndDraw(
      uint64_t ticksSinceLastCall,
      std::function<void()> clearFbFunction,
      uint8_t kdown,
      uint8_t kheld);

    void FlipBuffer_PP(uint8_t* fb, int width, int height, std::function<void()> postFlipFunction);

    void FlipBuffer_STF(uint8_t* fb, int width, int height, std::function<void()> postFlipFunction);

    void FlipBuffer_SAO(
      uint8_t* fb, int width, int height, 
      uint8_t* fb_o, int width_o, int height_o, 
      std::function<void()> postFlipFunction);

    void FillAudioBuffer(void *audioBuffer,size_t offset, size_t size);

    void TurnOff();

    uint8_t GetTargetFps();

    int GetFrameCount();
};


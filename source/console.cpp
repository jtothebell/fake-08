#include <string>
#include <functional>

#include "console.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"
#include "picoluaapi.h"
#include "logger.h"
#include "Input.h"
#include "p8GlobalLuaFunctions.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

Console::Console(){
    Logger::Write("getting font string\n");
    auto fontdata = get_font_data();

    Logger::Write("Creating Graphics object\n");
    Graphics* graphics = new Graphics(fontdata);
    _graphics = graphics;

    Logger::Write("Creating Input object\n");
    Input* input = new Input();
    _input = input;

    //this can probably go away when I'm loading actual carts and just have to expose api to lua
    Logger::Write("Initializing global api\n");
    initPicoApi(_graphics, _input, this);
    //initGlobalApi(_graphics);

    _targetFps = 30;
}

Console::~Console(){
    delete _graphics;
    delete _input;

    if (_loadedCart){
        delete _loadedCart;
    }
}

void Console::LoadCart(std::string filename){
    Logger::Write("Calling Cart Constructor\n");
    Cart *cart = new Cart(filename);
    _picoFrameCount = 0;

    _graphics->setSpriteSheet(cart->SpriteSheetString);
    _graphics->setSpriteFlags(cart->SpriteFlagsString);
    _graphics->setMapData(cart->MapString);

    _loadedCart = cart;

    
    // initialize Lua interpreter
    _luaState = luaL_newstate();

    // load Lua base libraries (print / math / etc)
    luaL_openlibs(_luaState);

    //load in global lua fuctions for pico 8
    int loadedGlobals = luaL_dostring(_luaState, p8GlobalLuaFunctions);

    if (loadedGlobals != LUA_OK) {
        Logger::Write("ERROR loading pico 8 lua globals\n");
        Logger::Write("Error: %s\n", lua_tostring(_luaState, -1));
        lua_pop(_luaState, 1);

        return;
    }

    //graphics
    lua_register(_luaState, "cls", cls);
    lua_register(_luaState, "pset", pset);
    lua_register(_luaState, "pget", pget);
    lua_register(_luaState, "color", color);
    lua_register(_luaState, "line", line);
    lua_register(_luaState, "circ", circ);
    lua_register(_luaState, "circfill", circfill);
    lua_register(_luaState, "rect", rect);
    lua_register(_luaState, "rectfill", rectfill);
    lua_register(_luaState, "print", print);
    lua_register(_luaState, "spr", spr);
    lua_register(_luaState, "sspr", sspr);
    lua_register(_luaState, "fget", fget);
    lua_register(_luaState, "fset", fset);
    lua_register(_luaState, "sget", sget);
    lua_register(_luaState, "sset", sset);
    lua_register(_luaState, "camera", camera);
    lua_register(_luaState, "clip", clip);

    lua_register(_luaState, "pal", pal);
    lua_register(_luaState, "palt", palt);

    lua_register(_luaState, "mget", mget);
    lua_register(_luaState, "mset", mset);
    lua_register(_luaState, "map", map);

    //stubbed in graphics:
    lua_register(_luaState, "fillp", fillp);
    lua_register(_luaState, "flip", flip);

    //input
    lua_register(_luaState, "btn", btn);
    lua_register(_luaState, "btnp", btnp);

    lua_register(_luaState, "time", time);
    lua_register(_luaState, "t", time);

    //stubbed in audio:
    lua_register(_luaState, "music", music);
    lua_register(_luaState, "sfx", sfx);

    //stubbed in memory
    lua_register(_luaState, "cstore", cstore);
    lua_register(_luaState, "memcpy", memcpy);
    lua_register(_luaState, "memset", memset);
    lua_register(_luaState, "peek", peek);
    lua_register(_luaState, "poke", poke);
    lua_register(_luaState, "reload", reload);

    //stubbed in cart data
    lua_register(_luaState, "cartdata", cartdata);
    lua_register(_luaState, "dget", dget);
    lua_register(_luaState, "dset", dset);

    int loadedCart = luaL_dostring(_luaState, _loadedCart->LuaString.c_str());

    if (loadedCart != LUA_OK) {
        Logger::Write("ERROR loading cart\n");
        Logger::Write("Error: %s\n", lua_tostring(_luaState, -1));
        lua_pop(_luaState, 1);

        return;
    }


    // Push the _init function on the top of the lua stack (or nil if it doesn't exist)
    lua_getglobal(_luaState, "_init");

    if (lua_isfunction(_luaState, -1)) {
        lua_call(_luaState, 0, 0);
    }

    //pop the _init fuction off the stack now that we're done with it
    lua_pop(_luaState, 0);

    //check for update, mark correct target fps
    lua_getglobal(_luaState, "_update");
    if (lua_isfunction(_luaState, -1)) {
        _hasUpdate = true;
        _targetFps = 30;
    }
    lua_pop(_luaState, 0);
    if (!_hasUpdate){
        lua_getglobal(_luaState, "_update60");
        if (lua_isfunction(_luaState, -1)) {
            _hasUpdate = true;
            _targetFps = 60;
        }
        lua_pop(_luaState, 0);
    }

    lua_getglobal(_luaState, "_draw");
    if (lua_isfunction(_luaState, -1)) {
        _hasDraw = true;
    }
    lua_pop(_luaState, 0);
}


//how to call lua from c: https://www.cs.usfca.edu/~galles/cs420/lecture/LuaLectures/LuaAndC.html
void Console::UpdateAndDraw(
      uint64_t ticksSinceLastCall,
      std::function<void()> clearFbFunction,
      uint8_t kdown,
      uint8_t kheld)
{
    if (clearFbFunction) {
        clearFbFunction();
    }

    _input->SetState(kdown, kheld);

    if (_hasUpdate){
        // Push the _update function on the top of the lua stack
        if (_targetFps == 60) {
            lua_getglobal(_luaState, "_update60");
        } else {
            lua_getglobal(_luaState, "_update");
        }

        //we already checked that its a function, so we should be able to call it
        lua_call(_luaState, 0, 0);

        //pop the update fuction off the stack now that we're done with it
        lua_pop(_luaState, 0);
    }

    if (_hasDraw) {
        lua_getglobal(_luaState, "_draw");

        lua_call(_luaState, 0, 0);
        
        //pop the update fuction off the stack now that we're done with it
        lua_pop(_luaState, 0);
    }

    _picoFrameCount++;
}

void Console::FlipBuffer(uint8_t* fb, int width, int height, std::function<void()> postFlipFunction){
    _graphics->flipBuffer(fb, width, height);

    if (postFlipFunction) {
        postFlipFunction();
    }
}

void Console::TurnOff() {
    lua_close(_luaState);
}

uint8_t Console::GetTargetFps() {
    return _targetFps;
}

int Console::GetFrameCount() {
    return _picoFrameCount;
}

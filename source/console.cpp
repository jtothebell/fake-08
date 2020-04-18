#include <string>

#include "console.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"
#include "picoluaapi.h"
#include "logger.h"
#include "Input.h"

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
    initPicoApi(_graphics, _input);
    //initGlobalApi(_graphics);
}

void Console::LoadCart(std::string filename){
    Logger::Write("Calling Cart Constructor\n");
    Cart cart = Cart(filename);

    _loadedCart = &cart;

    _graphics->setSpriteSheet(_loadedCart->SpriteSheetString);

    // initialize Lua interpreter
    _luaState = luaL_newstate();

    // load Lua base libraries (print / math / etc)
    luaL_openlibs(_luaState);

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
    lua_register(_luaState, "btn", btn);
    lua_register(_luaState, "btnp", btnp);

    luaL_dostring(_luaState, _loadedCart->LuaString.c_str());
}

void Console::UpdateAndDraw(int frameCount, uint8_t kdown, uint8_t kheld){
    _input->SetState(kdown, kheld);

    // Push the fib function on the top of the lua stack
    lua_getglobal(_luaState, "_update");

    // Push any arguments (the number 13 in this case) on the stack 
    //lua_pushnumber(_luaState, 13);

    // call the function with 0 argument, returning 0 resulta.  Note that the function actually
    // returns 2 results -- we just don't want them
    lua_call(_luaState, 0, 0);

    // Get the result from the lua stack
    //int result = (int)lua_tointeger(_luaState, -1);

    // Clean up.  If we don't do this last step, we'll leak stack memory.
    lua_pop(_luaState, 0);


    lua_getglobal(_luaState, "_draw");
    lua_call(_luaState, 0, 0);
    lua_pop(_luaState, 0);

}

void Console::FlipBuffer(uint8_t* fb){
    _graphics->flipBuffer(fb);
}

void Console::TurnOff() {
    lua_close(_luaState);
}

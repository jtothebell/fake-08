#include <string>
#include <functional>
#include <math.h>

#include <string.h>

#include "vm.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"
#include "picoluaapi.h"
#include "logger.h"
#include "Input.h"
#include "p8GlobalLuaFunctions.h"
#include "hostVmShared.h"
#include "emojiconversion.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}

Vm::Vm(){
    Logger::Write("getting font string\n");
    auto fontdata = get_font_data();

    _memory = {0};
    

    Logger::Write("Creating Graphics object\n");
    Graphics* graphics = new Graphics(fontdata, &_memory);
    _graphics = graphics;

    _memory._gfxState_color = 7;
    _graphics->clip();
    _graphics->pal();

    Logger::Write("Creating Input object\n");
    Input* input = new Input();
    _input = input;

    Audio* audio = new Audio(&_memory);
    _audio = audio;

    //this can probably go away when I'm loading actual carts and just have to expose api to lua
    Logger::Write("Initializing global api\n");
    initPicoApi(_graphics, _input, this, _audio);
    //initGlobalApi(_graphics);

    _targetFps = 30;
}

Vm::~Vm(){
    CloseCart();

    delete _graphics;
    delete _input;
    delete _audio;
}

bool Vm::loadCart(Cart* cart) {
    _picoFrameCount = 0;

    //reset memory (may have to be more selective about zeroing out to be accurate?)
    _memory = {0};
    //set graphics state
    _memory._gfxState_color = 7;
    _graphics->clip();
    _graphics->pal();

    if (cart->LuaString == "") {
        if (_cartLoadError == "") {
            _cartLoadError = "No Lua to load. Aborting cart load";
        }
        Logger::Write("%s\n", _cartLoadError.c_str());

        return false;
    }

    //reset audio
    for(int i = 0; i < 4; i++) {
        _memory._sfxChannels[i].sfxId = -1;
    }
    _memory._musicChannel.pattern = -1;

    //copy data from cart rom to ram
    for(size_t i = 0; i < sizeof(_memory.spriteSheetData); i++) {
        _memory.spriteSheetData[i] = cart->SpriteSheetData[i];
    }
    for(size_t i = 0; i < sizeof(_memory.spriteFlags); i++) {
        _memory.spriteFlags[i] = cart->SpriteFlagsData[i];
    }
    for(size_t i = 0; i < sizeof(_memory.mapData); i++) {
        _memory.mapData[i] = cart->MapData[i];
    }

    for(size_t i = 0; i < 64; i++) {
        _memory.sfx[i] = cart->SfxData[i];
    }

    for(size_t i = 0; i < 64; i++) {
        _memory.songs[i] = cart->SongData[i];
    }

    // initialize Lua interpreter
    _luaState = luaL_newstate();

    // load Lua base libraries (print / math / etc)
    luaL_openlibs(_luaState);

    //load in global lua fuctions for pico 8
    auto convertedGlobalLuaFunctions = convert_emojis(p8GlobalLuaFunctions);
    int loadedGlobals = luaL_dostring(_luaState, convertedGlobalLuaFunctions.c_str());

    if (loadedGlobals != LUA_OK) {
        _cartLoadError = "ERROR loading pico 8 lua globals";
        Logger::Write("ERROR loading pico 8 lua globals\n");
        Logger::Write("Error: %s\n", lua_tostring(_luaState, -1));
        lua_pop(_luaState, 1);

        return false;
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
    lua_register(_luaState, "cursor", cursor);
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
    lua_register(_luaState, "memcpy", api_memcpy);
    lua_register(_luaState, "memset", api_memset);
    lua_register(_luaState, "peek", peek);
    lua_register(_luaState, "poke", poke);
    lua_register(_luaState, "reload", reload);

    //stubbed in cart data
    lua_register(_luaState, "cartdata", cartdata);
    lua_register(_luaState, "dget", dget);
    lua_register(_luaState, "dset", dset);

    //system
    lua_register(_luaState, "__listcarts", listcarts);
    lua_register(_luaState, "__loadcart", loadcart);
    lua_register(_luaState, "__getbioserror", getbioserror);
    lua_register(_luaState, "__loadbioscart", loadbioscart);

    int loadedCart = luaL_dostring(_luaState, cart->LuaString.c_str());

    if (loadedCart != LUA_OK) {
        _cartLoadError = "Error loading cart lua";
        Logger::Write("ERROR loading cart\n");
        Logger::Write("Error: %s\n", lua_tostring(_luaState, -1));
        lua_pop(_luaState, 1);

        return false;
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

    _loadedCart = cart;

    _cartLoadError = "";

    return true;
}

void Vm::LoadBiosCart(){
    CloseCart();

    Cart *cart = new Cart("__FAKE08-BIOS.p8");

    bool success = loadCart(cart);

    if (!success) {
        CloseCart();
    }
}

void Vm::LoadCart(std::string filename){
    Logger::Write("Loading cart %s\n", filename.c_str());
    CloseCart();

    Logger::Write("Calling Cart Constructor\n");
    Cart *cart = new Cart(filename);

    _cartLoadError = cart->LoadError;

    bool success = loadCart(cart);

    if (!success) {
        CloseCart();
        //todo: show an error message on the bios?
        LoadBiosCart();
    }
}


//how to call lua from c: https://www.cs.usfca.edu/~galles/cs420/lecture/LuaLectures/LuaAndC.html
void Vm::UpdateAndDraw(
      uint8_t kdown,
      uint8_t kheld)
{
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

    //todo: pause menu here, but for now just load bios
    if (kdown & P8_KEY_PAUSE) {
        QueueCartChange("__FAKE08-BIOS.p8");
    }

    if (_cartChangeQueued) {
        LoadCart(_nextCartKey);

        _cartChangeQueued = false;
    }
}

uint8_t* Vm::GetPicoInteralFb(){
    return _graphics->GetP8FrameBuffer();
}

uint8_t* Vm::GetScreenPaletteMap(){
    return _graphics->GetScreenPaletteMap();
}

Color* Vm::GetPaletteColors(){
    return _graphics->GetPaletteColors();
}


void Vm::FillAudioBuffer(void *audioBuffer, size_t offset, size_t size){
   _audio->FillAudioBuffer(audioBuffer, offset, size);
}

void Vm::CloseCart() {
    if (_loadedCart){
        Logger::Write("deleting cart\n");
        delete _loadedCart;
        _loadedCart = nullptr;
    }
    
    if (_luaState) {
        Logger::Write("closing lua state\n");
        lua_close(_luaState);
        _luaState = nullptr;
    }

    Logger::Write("resetting state\n");
    _hasUpdate = false;
    _hasDraw = false;
    _targetFps = 30;
    _picoFrameCount = 0;
}

void Vm::QueueCartChange(std::string filename){
    _nextCartKey = filename;
    _cartChangeQueued = true;
}

int Vm::GetTargetFps() {
    return _targetFps;
}

int Vm::GetFrameCount() {
    return _picoFrameCount;
}

void Vm::SetCartList(vector<string> cartList){
    _cartList = cartList;
}

vector<string> Vm::GetCartList(){
    return _cartList;
}

string Vm::GetBiosError() {
    return _cartLoadError;
}

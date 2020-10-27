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

//extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
//}

Vm::Vm(
    PicoRam* memory,
    Graphics* graphics,
    Input* input,
    Audio* audio) :
        _loadedCart(nullptr),
        _luaState(nullptr),
        _cleanupDeps(false),
        _targetFps(30),
        _picoFrameCount(0),
        _hasUpdate(false),
        _hasDraw(false),
        _cartChangeQueued(false),
        _nextCartKey(""),
        _cartLoadError(""),
        _cartdataKey("")
{
    if (memory == nullptr) {
        memory = new PicoRam();
        _cleanupDeps = true;
    }
    _memory = memory;
    
    if (graphics == nullptr) {
        graphics = new Graphics(get_font_data(), _memory);
        _cleanupDeps = true;
    }
    _graphics = graphics;

    if (input == nullptr){
        input = new Input(memory);
        _cleanupDeps = true;
    }
    _input = input;

    if (audio == nullptr) {
        audio = new Audio(_memory);
        _cleanupDeps = true;
    }
    _audio = audio;

    //this can probably go away when I'm loading actual carts and just have to expose api to lua
    Logger::Write("Initializing global api\n");
    initPicoApi(_graphics, _input, this, _audio);
    //initGlobalApi(_graphics);

}

Vm::~Vm(){
    CloseCart();

    if (_cleanupDeps){
        if (_input != nullptr) {
            delete _input;
        }
        if (_graphics != nullptr) {
            delete _graphics;
        }
        if (_audio != nullptr) {
            delete _audio;
        }
        if (_memory != nullptr) {
            delete _memory;
        }
    }
}

PicoRam* Vm::getPicoRam(){
    return _memory;
}

bool Vm::loadCart(Cart* cart) {
    _picoFrameCount = 0;
    _cartdataKey = "";

    //reset memory (may have to be more selective about zeroing out to be accurate?)
    _memory->Reset();

    //set graphics state
    _graphics->color();
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
    _audio->resetAudioState();

    //copy data from cart rom to ram
    vm_reload(0, 0, sizeof(cart->CartRom), cart);

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

    //audio:
    lua_register(_luaState, "music", music);
    lua_register(_luaState, "sfx", sfx);

    //memory
    lua_register(_luaState, "cstore", cstore);
    lua_register(_luaState, "memcpy", api_memcpy);
    lua_register(_luaState, "memset", api_memset);
    lua_register(_luaState, "peek", peek);
    lua_register(_luaState, "poke", poke);
    lua_register(_luaState, "peek2", peek2);
    lua_register(_luaState, "poke2", poke2);
    lua_register(_luaState, "peek4", peek4);
    lua_register(_luaState, "poke4", poke4);
    lua_register(_luaState, "reload", reload);

    //cart data
    lua_register(_luaState, "cartdata", cartdata);
    lua_register(_luaState, "dget", dget);
    lua_register(_luaState, "dset", dset);

    //
    lua_register(_luaState, "printh", printh);
    lua_register(_luaState, "stat", stat);

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

bool Vm::ExecuteLua(string luaString, string callbackFunction){
    int success = luaL_dostring(_luaState, luaString.c_str());

    if (! success == LUA_OK){
        //bad lua passed
        Logger::Write("Error: %s\n", lua_tostring(_luaState, -1));
        lua_pop(_luaState, 1);

        return false;
    }

    if (callbackFunction.length() > 0) {
        lua_getglobal(_luaState, callbackFunction.c_str());
        lua_call(_luaState, 0, 1);

        bool result = lua_toboolean(_luaState, -1);

        lua_pop(_luaState, 0);

        return result;
    }

    return true;
}


uint8_t Vm::vm_peek(int addr){
    if (addr < 0 || addr > 0x8000){
        return 0;
    }

    return _memory->data[addr];
}

int16_t Vm::vm_peek2(int addr){
    //zepto8
    int16_t bits = 0;
    for (int i = 0; i < 2; ++i)
    {
        /* This code handles partial reads by adding zeroes */
        if (addr + i < 0x8000)
            bits |= _memory->data[addr + i] << (8 * i);
        else if (addr + i >= 0x8000)
            bits |= _memory->data[addr + i - 0x8000] << (8 * i);
    }

    return bits;
}

//note: this should return a 32 bit fixed point number
int32_t Vm::vm_peek4(int addr){
    //zepto8
    int32_t bits = 0;
    for (int i = 0; i < 4; ++i)
    {
        /* This code handles partial reads by adding zeroes */
        if (addr + i < 0x8000)
            bits |= _memory->data[addr + i] << (8 * i);
        else if (addr + i >= 0x8000)
            bits |= _memory->data[addr + i - 0x8000] << (8 * i);
    }

    return bits;
} 

void Vm::vm_poke(int addr, uint8_t value){
    //todo: check how pico 8 handles out of bounds
    if (addr < 0 || addr > 0x8000){
        return;
    }
    
    _memory->data[addr] = value;
}

void Vm::vm_poke2(int addr, int16_t value){
    if (addr < 0 || addr > 0x8000 - 1){
        return;
    }

    _memory->data[addr] = (uint8_t)value;
    _memory->data[addr + 1] = (uint8_t)(value >> 8);

}

void Vm::vm_poke4(int addr, int32_t value){
    if (addr < 0 || addr > 0x8000 - 3){
        return;
    }

    _memory->data[addr + 0] = (uint8_t)value;
    _memory->data[addr + 1] = (uint8_t)(value >> 8);
    _memory->data[addr + 2] = (uint8_t)(value >> 16);
    _memory->data[addr + 3] = (uint8_t)(value >> 24);
}

void Vm::vm_cartdata(string key) {
    _cartdataKey = key;
}

int32_t Vm::vm_dget(uint8_t n) {
    if (_cartdataKey.length() > 0 && n < 64) {
        return vm_peek4(0x5e00 + 4 * n);
    }

    return 0;
}

void Vm::vm_dset(uint8_t n, int32_t value){
    if (_cartdataKey.length() > 0 && n < 64) {
        vm_poke4(0x5e00 + 4 * n, value);
    }
}

void Vm::vm_reload(int destaddr, int sourceaddr, int len, Cart* cart){
    memcpy(&_memory->data[destaddr], &cart->CartRom.data[sourceaddr], sizeof(cart->CartRom));
}

void Vm::vm_reload(int destaddr, int sourceaddr, int len, string filename){
    if (destaddr < 0 || destaddr > (int)sizeof(PicoRam)) {
        //invalid dest address
        return;
    }
    if (sourceaddr < 0 || sourceaddr > (int)sizeof(CartRomData)) {
        //invalid source address
        return;
    }
    if (len < 0 || destaddr + len > (int)sizeof(CartRomData)) {
        //invalid length address
        return;
    }

    Cart* cart = _loadedCart;
    bool multicart = false;

    if (filename.length() > 0) {
        cart = new Cart(filename);
        if (cart->LoadError.length() > 0) {
            //error, can't load cart
            //todo: see what kind of error pico 8 throws, emulate
            delete cart;

            return;
        }

        multicart = true;
    }

    vm_reload(destaddr, sourceaddr, len, cart);

    if (multicart) {
        delete cart;
    }
}

void Vm::vm_memset(int destaddr, uint8_t val, int len){
    if (destaddr < 0 || destaddr + len > (int)sizeof(PicoRam)) {
        return;
    }

    memset(&_memory->data[destaddr], val, len);

}
void Vm::vm_memcpy(int destaddr, int sourceaddr, int len){
    if (sourceaddr < 0 || sourceaddr + len > (int)sizeof(PicoRam)) {
        return;
    }
    if (destaddr < 0 || destaddr + len > (int)sizeof(PicoRam)) {
        return;
    }

    memcpy(&_memory->data[destaddr], &_memory->data[sourceaddr], len);
}

#include <string>
#include <functional>
#include <chrono>
#include <math.h>
#include <setjmp.h>
#include <algorithm>

#include <string.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>

#include "vm.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"
#include "stringToDataHelpers.h"
#include "picoluaapi.h"
#include "logger.h"
#include "Input.h"
#include "p8GlobalLuaFunctions.h"
#include "hostVmShared.h"
#include "emojiconversion.h"

#include "NoLabel.h"

//extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
  #include <fix32.h>
//}

using namespace z8;

static const char BiosCartName[] = "__FAKE08-BIOS.p8";
static const char SettingsCartName[] = "__FAKE08-SETTINGS.p8";

Vm::Vm(
    Host* host,
    PicoRam* memory,
    Graphics* graphics,
    Input* input,
    Audio* audio) :
        _loadedCart(nullptr),
        _luaState(nullptr),
        _cleanupDeps(false),
        _targetFps(30),
        _picoFrameCount(0),
        _cartChangeQueued(false),
        _nextCartKey(""),
        _cartLoadError(""),
        _cartdataKey("")
{
    _host = host;

    if (memory == nullptr) {
        memory = new PicoRam();
        _cleanupDeps = true;
    }
    _memory = memory;

    _pauseMenu = false;
    memset(_drawStateCopy, 0, sizeof(drawState_t));
    
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
    Logger_Write("Initializing global api\n");
    initPicoApi(_memory, _graphics, _input, this, _audio);
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

jmp_buf place;
bool abortLua;

bool Vm::loadCart(Cart* cart) {
    _picoFrameCount = 0;

    if (_cartdataKey.length() > 0) {
        _host->saveCartData(_cartdataKey, getSerializedCartData());
    }

    _cartdataKey = "";

    //reset memory (may have to be more selective about zeroing out to be accurate?)
    _memory->Reset();

    //seed rng
    auto now = std::chrono::high_resolution_clock::now();
    api_srand(fix32::frombits((int32_t)now.time_since_epoch().count()));

    //set graphics state
    _graphics->color();
    _graphics->clip();
    _graphics->pal();

    if (cart->LuaString == "") {
        if (_cartLoadError == "") {
            _cartLoadError = "No Lua to load. Aborting cart load";
        }
        Logger_Write("%s\n", _cartLoadError.c_str());

        return false;
    }

    //reset audio
    _audio->resetAudioState();

    //copy data from cart rom to ram
    vm_reload(0, 0, sizeof(cart->CartRom), cart);

    _loadedCart = cart;
    _cartChangeQueued = false;
    abortLua = false;

    // initialize Lua interpreter
    _luaState = luaL_newstate();

    lua_setpico8memory(_luaState, (uint8_t *)&_memory->data);
    // load Lua base libraries (print / math / etc)
    luaL_openlibs(_luaState);
    lua_pushglobaltable(_luaState);

    //system
    //must be registered before loading globals for pause menu to work
    lua_register(_luaState, "__listcarts", listcarts);
    lua_register(_luaState, "__getbioserror", getbioserror);
    lua_register(_luaState, "__loadbioscart", loadbioscart);
    lua_register(_luaState, "__loadsettingscart", loadsettingscart);
    lua_register(_luaState, "__togglepausemenu", togglepausemenu);
    lua_register(_luaState, "__resetcart", resetcart);
    lua_register(_luaState, "load", load);
	
    //settings
    lua_register(_luaState, "__getsetting", getsetting);
    lua_register(_luaState, "__setsetting", setsetting);
    
    lua_register(_luaState, "__installpackins", installpackins);
    
    //label
    lua_register(_luaState, "__loadlabel", loadlabel);
    
    lua_register(_luaState, "__getlualine", getlualine);
    
    //register global functions first, they will get local aliases when
    //the rest of the api is registered
    //graphics
    lua_register(_luaState, "cls", cls);
    lua_register(_luaState, "pset", pset);
    lua_register(_luaState, "pget", pget);
    lua_register(_luaState, "color", color);
    lua_register(_luaState, "line", line);
    lua_register(_luaState, "tline", tline);
    lua_register(_luaState, "circ", circ);
    lua_register(_luaState, "circfill", circfill);
    lua_register(_luaState, "oval", oval);
    lua_register(_luaState, "ovalfill", ovalfill);
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
    lua_register(_luaState, "map", gfx_map);
    lua_register(_luaState, "mapdraw", gfx_map);

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
    lua_register(_luaState, "reset", reset);

    //cart data
    lua_register(_luaState, "cartdata", cartdata);
    lua_register(_luaState, "dget", dget);
    lua_register(_luaState, "dset", dset);

    //
    lua_register(_luaState, "printh", printh);
    lua_register(_luaState, "stat", stat);
    lua_register(_luaState, "_update_buttons", _update_buttons);
    lua_register(_luaState, "run", run);
    lua_register(_luaState, "extcmd", extcmd);

    //rng
    lua_register(_luaState, "rnd", rnd);
    lua_register(_luaState, "srand", srand);

    //load in global lua fuctions for pico 8- part of this is setting a local variable
    //with the same name as all the globals we just registered
    //auto convertedGlobalLuaFunctions = convert_emojis(p8GlobalLuaFunctions);
    auto convertedGlobalLuaFunctions = charset::utf8_to_pico8(p8GlobalLuaFunctions);
    int loadedGlobals = luaL_dostring(_luaState, convertedGlobalLuaFunctions.c_str());

    if (loadedGlobals != LUA_OK) {
        _cartLoadError = "ERROR loading pico 8 lua globals";
        Logger_Write("ERROR loading pico 8 lua globals\n");
        Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
        lua_pop(_luaState, 1);

        return false;
    }


    int loadedCart = luaL_loadstring(_luaState, cart->LuaString.c_str());
    if (loadedCart != LUA_OK) {
        _cartLoadError = "Error loading cart lua:\n";
        _cartLoadError.append(lua_tostring(_luaState, -1));
        Logger_Write(_cartLoadError.c_str());
        lua_pop(_luaState, 1);

        return false;
    }

    if (setjmp(place) == 0) {
        if (lua_pcall(_luaState, 0, 0, 0)){
            _cartLoadError = "Runtime error";
            Logger_Write("ERROR running cart\n");
            Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
            lua_pop(_luaState, 1);
            return false;
        }
    }

    if (abortLua) {
        //trigger closing of cart and reload of bios
        return false;
    }
    
    #if LOAD_PACK_INS
    if(cart->FullCartPath == SettingsCartName){
        
        std::string enablepackins = "showpackinoptions = true";
        int doStrRes = luaL_dostring(_luaState, enablepackins.c_str());

        if (doStrRes != LUA_OK){
            //bad lua passed
            Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
            lua_pop(_luaState, 1);
        }
        
        
    }
    
    #endif

    // Push the _init function on the top of the lua stack (or nil if it doesn't exist)
    lua_getglobal(_luaState, "_init");

    if (lua_isfunction(_luaState, -1)) {
        if (lua_pcall(_luaState, 0, 0, 0)){
            _cartLoadError = lua_tostring(_luaState, -1);
            Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
            lua_pop(_luaState, 1);
            QueueCartChange(BiosCartName);
            return false;
        }
    }

    //pop the _init fuction off the stack now that we're done with it
    lua_pop(_luaState, 0);

    //check for update, mark correct target fps
    lua_getglobal(_luaState, "_update60");
    if (lua_isfunction(_luaState, -1)) {
        _targetFps = 60;
    }
    else {
        _targetFps = 30;
    }
    lua_pop(_luaState, 0);


    //customize bios per host's requirements
    if (cart->FullCartPath == BiosCartName || cart->FullCartPath == SettingsCartName) {
        std::string customBiosLua = _host->customBiosLua();

        if (customBiosLua.length() > 0) {
            int doStrRes = luaL_dostring(_luaState, customBiosLua.c_str());

            if (doStrRes != LUA_OK){
                //bad lua passed
                Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
                lua_pop(_luaState, 1);
            }
        }
    }

    if (_cartBreadcrumb.length() > 0) {
        ExecuteLua("__addbreadcrumb(\"" + _cartBreadcrumb +"\", \"" + _prevCartKey +"\")", "");
    }

    _cartLoadError = "";

    return true;
}

void Vm::LoadBiosCart(){
    CloseCart();
    Cart *cart = new Cart(BiosCartName, "");

    bool success = loadCart(cart);

    if (!success) {
        CloseCart();
    }
}

void Vm::LoadSettingsCart(){
    CloseCart();

    Cart *cart = new Cart(SettingsCartName, "");

    bool success = loadCart(cart);

    if (!success) {
        CloseCart();
    }
}

void Vm::LoadCart(std::string filename, bool loadBiosOnFail){
    if (filename == "__FAKE08-BIOS.p8") {
        LoadBiosCart();
        return;
    }
    if (filename == "__FAKE08-SETTINGS.p8") {
        LoadSettingsCart();
        return;
    }
    Logger_Write("Loading cart %s\n", filename.c_str());
    CloseCart();

    Logger_Write("Calling Cart Constructor\n");
    auto cartDir = _host->getCartDirectory();
    Cart *cart = new Cart(filename, cartDir);

    _cartLoadError = cart->LoadError;

    bool success = loadCart(cart);

    if (loadBiosOnFail && !success) {
        CloseCart();
        //todo: show an error message on the bios?
        LoadBiosCart();
    }
}

void Vm::togglePauseMenu(){
    if (_memory->drawState.suppressPause) {
        _memory->drawState.suppressPause = 0;
        return;
    }

    _pauseMenu = !_pauseMenu;

    if (_pauseMenu){
        //save old draw state
        //0x5f00-0x5f3f - 64 bytes
        memcpy(_drawStateCopy, &_memory->drawState, 64);

        _graphics->pal();
        _graphics->fillp(0);
        _graphics->clip();
        _graphics->camera();
        _graphics->color();
    }
    else{
        //restore old draw state
        memcpy(&_memory->drawState, _drawStateCopy, 64);
    }
    
}


//https://stackoverflow.com/a/30606613
std::vector<uint8_t> HexToBytes(std::string hex) {
  std::vector<uint8_t> bytes;

  hex.erase(std::remove(hex.begin(), hex.end(), '\n'), hex.end());

  for (unsigned int i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    uint8_t byte = (uint8_t) strtol(byteString.c_str(), NULL, 16);
    bytes.push_back(byte);
  }

  return bytes;
}

std::string Vm::getSerializedCartData() {
    std::string outputstr = "";

    char hex_string[9] = "00000000";

    //ATTN: writing one byte at a time instead of one 32 bit int at a time
    //to ensure same behavior across platforms and cpu architectures
    for(int i = 0; i < 64; i++){
        for(int b = 3; b >= 0; b--){
            uint8_t byte = _memory->data[0x5e00 + (i*4) + b];

            sprintf(hex_string, "%02x", byte);

            outputstr.append(hex_string);
        }

        if ((i + 1) % 8 == 0) {
            outputstr.append("\n");
        }
    }

    return outputstr;
}

void Vm::deserializeCartDataToMemory(std::string cartDataStr) {
    //populate from string (assume correct length? TODO: validation)
    std::vector<uint8_t> bytesVector = HexToBytes(cartDataStr);

    //ATTN: writing one byte at a time instead of one 32 bit int at a time
    //to ensure same behavior across platforms and cpu architectures
    for(size_t i = 0; i < 64; i++){
        size_t idxStart = (i*4);
        size_t idxEnd = idxStart + 3;
        if (idxEnd < bytesVector.size()){
            _memory->data[0x5e00 + idxStart + 0] = bytesVector[idxEnd];
            _memory->data[0x5e00 + idxStart + 1] = bytesVector[idxEnd - 1];
            _memory->data[0x5e00 + idxStart + 2] = bytesVector[idxEnd - 2];
            _memory->data[0x5e00 + idxStart + 3] = bytesVector[idxEnd - 3];
        }
    }

}

void Vm::UpdateAndDraw() {
    update_buttons();

    _picoFrameCount++;

    if (_input->btnp(6)) {
        togglePauseMenu();
    }

    if (_cartChangeQueued) {
        _prevCartKey = CurrentCartFilename();
        LoadCart(_nextCartKey);
    }

    if (_pauseMenu){

        lua_getglobal(_luaState, "__f08_menu_update");
        lua_call(_luaState, 0, 0);
        lua_pop(_luaState, 0);

        lua_getglobal(_luaState, "__f08_menu_draw");
        lua_call(_luaState, 0, 0);
        lua_pop(_luaState, 0);
    }
    else{
        // Push the _update function on the top of the lua stack
        if (_targetFps == 60) {
            lua_getglobal(_luaState, "_update60");
        } else {
            lua_getglobal(_luaState, "_update");
        }

        if (lua_isfunction(_luaState, -1)) {
            if (lua_pcall(_luaState, 0, 0, 0)){
                _cartLoadError = lua_tostring(_luaState, -1);
                Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
                lua_pop(_luaState, 1);
                QueueCartChange(BiosCartName);
                return;
            }
        }
        //pop the update fuction off the stack now that we're done with it
        lua_pop(_luaState, 0);

        lua_getglobal(_luaState, "_draw");
        if (lua_isfunction(_luaState, -1)) {
            if (lua_pcall(_luaState, 0, 0, 0)){
                _cartLoadError = lua_tostring(_luaState, -1);
                Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
                lua_pop(_luaState, 1);
                QueueCartChange(BiosCartName);
                return;
            }
        }
        lua_pop(_luaState, 0);
    }

}

uint8_t* Vm::GetPicoInteralFb(){
    return _graphics->GetP8FrameBuffer();
}

uint8_t* Vm::GetScreenPaletteMap(){
    return _graphics->GetScreenPaletteMap();
}

void Vm::FillAudioBuffer(void *audioBuffer, size_t offset, size_t size){
   _audio->FillAudioBuffer(audioBuffer, offset, size);
}

void Vm::CloseCart() {
    if (_loadedCart){
        Logger_Write("deleting cart\n");
        delete _loadedCart;
        _loadedCart = nullptr;
    }
    
    if (_luaState) {
        Logger_Write("closing lua state\n");
        lua_close(_luaState);
        _luaState = nullptr;
    }

    Logger_Write("resetting state\n");
    _targetFps = 30;
    _picoFrameCount = 0;
}

void Vm::QueueCartChange(std::string filename){
    _nextCartKey = filename;
    _cartChangeQueued = true;
    _pauseMenu = false;
}

int Vm::GetTargetFps() {
    return _targetFps;
}

std::string Vm::CurrentCartFilename(){
    if (_loadedCart){
        return _loadedCart->FullCartPath;
    }

    return "";
}

int Vm::GetFrameCount() {
    return _picoFrameCount;
}

void Vm::SetCartList(vector<string> cartList){
    std::sort(cartList.begin(), cartList.end());
    _cartList = cartList;
}

vector<string> Vm::GetCartList(){
    return _cartList;
}

string Vm::GetBiosError() {
    return _cartLoadError;
}

void Vm::GameLoop() {
    while (_host->shouldRunMainLoop())
    {
        //shouldn't need to set this every frame
        _host->setTargetFps(_targetFps);

        //is this better at the end of the loop?
        _host->waitForTargetFps();

        if (_host->shouldQuit()) break; // break in order to return to hbmenu
        //this should probably be handled just in the host class
        _host->changeStretch();

        //update buttons needs to be callable from the cart, and also flip
        //it should update call the pico part of scanInput and set the values in memory
        //then we don't need to pass them in here
        UpdateAndDraw();

        uint8_t* picoFb = GetPicoInteralFb();
        uint8_t* screenPaletteMap = GetScreenPaletteMap();

        _host->drawFrame(picoFb, screenPaletteMap, _memory->drawState.drawMode);

        if (_host->shouldFillAudioBuff()) {
            FillAudioBuffer(_host->getAudioBufferPointer(), 0, _host->getAudioBufferSize());

            _host->playFilledAudioBuffer();
        }
    }
}

bool Vm::ExecuteLua(string luaString, string callbackFunction){
    int success = luaL_dostring(_luaState, luaString.c_str());

    if (success != LUA_OK){
        //bad lua passed
        Logger_Write("Error: %s\n", lua_tostring(_luaState, -1));
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

const int MemoryUpperBound = 0x10000;

uint8_t Vm::vm_peek(int addr){
    if (addr < 0 || addr > MemoryUpperBound){
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
        if (addr + i < MemoryUpperBound)
            bits |= _memory->data[addr + i] << (8 * i);
        else if (addr + i >= MemoryUpperBound)
            bits |= _memory->data[addr + i - MemoryUpperBound] << (8 * i);
    }

    return bits;
}

//note: this should return a 32 bit fixed point number
fix32 Vm::vm_peek4(int addr){
    //zepto8
    int32_t bits = 0;
    for (int i = 0; i < 4; ++i)
    {
        /* This code handles partial reads by adding zeroes */
        if (addr + i < MemoryUpperBound)
            bits |= _memory->data[addr + i] << (8 * i);
        else if (addr + i >= MemoryUpperBound)
            bits |= _memory->data[addr + i - MemoryUpperBound] << (8 * i);
    }

    return fix32::frombits(bits);
} 

void Vm::vm_poke(int addr, uint8_t value){
    //todo: check how pico 8 handles out of bounds
    if (addr < 0 || addr > MemoryUpperBound){
        return;
    }
    
    _memory->data[addr] = value;
}

void Vm::vm_poke2(int addr, int16_t value){
    if (addr < 0 || addr > MemoryUpperBound - 1){
        return;
    }

    _memory->data[addr] = (uint8_t)value;
    _memory->data[addr + 1] = (uint8_t)(value >> 8);

}

void Vm::vm_poke4(int addr, fix32 value){
    if (addr < 0 || addr > MemoryUpperBound - 3){
        return;
    }

    uint32_t ubits = (uint32_t)value.bits();
    _memory->data[addr + 0] = (uint8_t)ubits;
    _memory->data[addr + 1] = (uint8_t)(ubits >> 8);
    _memory->data[addr + 2] = (uint8_t)(ubits >> 16);
    _memory->data[addr + 3] = (uint8_t)(ubits >> 24);
}

bool Vm::vm_cartdata(string key) {
    //match pico 8 errors
    if (_cartdataKey != "") {
        QueueCartChange(BiosCartName);
        _cartLoadError = "cartdata() can only be called once";
        return false;
    }
    if (key.length() == 0 || key.length() > 64) {
        QueueCartChange(BiosCartName);
        _cartLoadError = "cart data id too long";
        return false;
    }
    //todo: validate chars

    _cartdataKey = key;

    auto cartDataStr = _host->getCartDataFileContents(_cartdataKey);

    //todo: validate hex format
    if (cartDataStr.length() > 0) {
        deserializeCartDataToMemory(cartDataStr);
        return true;
    } else {
        return false;
    }

    //call host to get current cart data and init- set memory
    //file name should match pico 8: {key}.p8d.txt in the cdata directory
    //call host to get that string if anything exists
}

fix32 Vm::vm_dget(uint8_t n) {
    if (n < 64) {
        return vm_peek4(0x5e00 + 4 * n);
    }

    return 0;
}

void Vm::vm_dset(uint8_t n, fix32 value){
    if (n < 64) {
        vm_poke4(0x5e00 + 4 * n, value);
    }
}

void Vm::vm_reload(int destaddr, int sourceaddr, int len, Cart* cart){
    if (len <= 0) {
        return;
    }
    memcpy(&_memory->data[destaddr], &cart->CartRom.data[sourceaddr], len);
}

void Vm::vm_reload(int destaddr, int sourceaddr, int len, string filename){
    if (len <= 0) {
        return;
    }
    if (destaddr < 0 || destaddr > (int)sizeof(PicoRam)) {
        //invalid dest address
        return;
    }
    if (sourceaddr < 0 || sourceaddr > (int)sizeof(CartRomData)) {
        //invalid source address
        return;
    }
    if (len < 0 || destaddr + len > (int)sizeof(PicoRam)) {
        //invalid length address
        return;
    }

    Cart* cart = _loadedCart;
    bool multicart = false;

    if (filename.length() > 0) {
        cart = new Cart(filename, _host->getCartDirectory());
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
    if (len <= 0) {
        return;
    }
    if (destaddr < 0 || destaddr + len > (int)sizeof(PicoRam)) {
        return;
    }

    memset(&_memory->data[destaddr], val, len);

}
void Vm::vm_memcpy(int destaddr, int sourceaddr, int len){
    if (len <= 0) {
        return;
    }
    if (sourceaddr < 0 || sourceaddr + len > (int)sizeof(PicoRam)) {
        return;
    }
    if (destaddr < 0 || destaddr + len > (int)sizeof(PicoRam)) {
        return;
    }

    memcpy(&_memory->data[destaddr], &_memory->data[sourceaddr], len);
}

void Vm::update_prng()
{
    auto rngState = _memory->hwState.rngState;
    rngState[1] = rngState[0] + ((rngState[1] >> 16) | (rngState[1] << 16));
    rngState[0] += rngState[1];
}

fix32 Vm::api_rnd()
{
    return api_rnd((fix32)1);
}

fix32 Vm::api_rnd(fix32 in_range)
{
    update_prng();
    uint32_t b = _memory->hwState.rngState[1];
    uint32_t range = in_range.bits();
    return fix32::frombits(range > 0 ? b % range : 0);
}

void Vm::api_srand(fix32 seed)
{
    auto rngState = _memory->hwState.rngState;
    rngState[0] = seed ? seed.bits() : 0xdeadbeef;
    rngState[1] = rngState[0] ^ 0xbead29ba;
    for (int i = 0; i < 32; ++i)
        update_prng();
}

void Vm::update_buttons() {
    //get button states from hardware
    auto inputState = _host->scanInput();
    _input->SetState(inputState.KDown, inputState.KHeld);
    if (_memory->drawState.devkitMode) {
        _input->SetMouse(inputState.mouseX, inputState.mouseY, inputState.mouseBtnState);
        _input->SetKeyboard(inputState.KBdown,inputState.KBkey);
    }
    else {
        _input->SetMouse(0, 0, 0);
        _input->SetKeyboard(false,"");
    }
}

void Vm::vm_flip() {
    if (!_host->shouldRunMainLoop()){
        abortLua = true;
        if (abortLua){
            longjmp(place, 1);
        }
    }

    if (!_host->shouldQuit() && !_cartChangeQueued) {
        update_buttons();

        _picoFrameCount++;

        //todo: pause menu here, but for now just load bios
        if (_input->btnp(6)) {
            //QueueCartChange(BiosCartName);
            togglePauseMenu();

            //shouldn't get here
            return;
        }

        _host->changeStretch();

        _host->setTargetFps(_targetFps);

        uint8_t* picoFb = GetPicoInteralFb();
        uint8_t* screenPaletteMap = GetScreenPaletteMap();

        if (_pauseMenu){
            //pause menu probably needs refactor out of lua. For now this is better than just quitting
            lua_getglobal(_luaState, "__f08_menu_update");
            lua_call(_luaState, 0, 0);
            lua_pop(_luaState, 0);

            lua_getglobal(_luaState, "__f08_menu_draw");
            lua_call(_luaState, 0, 0);
            lua_pop(_luaState, 0);
        }

        _host->drawFrame(picoFb, screenPaletteMap, _memory->drawState.drawMode);

        //is this better at the end of the loop?
        _host->waitForTargetFps();
    }
}

void Vm::vm_run() {
    if (_loadedCart) {
        loadCart(_loadedCart);
    }
}

void Vm::vm_extcmd(std::string cmd){
    //screenshots, recording, and breadcrumb not supported
    if (cmd == "reset"){
        QueueCartChange(CurrentCartFilename());
    }
    else if (cmd == "pause") {
        togglePauseMenu();
    }
    else if (cmd == "shutdown") {
        QueueCartChange(BiosCartName);
    }
}

void Vm::vm_load(std::string filename, std::string breadcrumb, std::string param){
    _cartBreadcrumb = breadcrumb;
    if (param.length() > 0) {
        _cartParam = param;
    }

    QueueCartChange(filename);
}

void Vm::vm_reset(){
    memset(&_memory->data[0x5f00], 0, 0x7f);

    _memory->hwState.colorBitmask = 0xff;
    _memory->hwState.spriteSheetMemMapping = 0x00;
    _memory->hwState.screenDataMemMapping = 0x60;
    _memory->hwState.mapMemMapping = 0x20;
    _memory->hwState.widthOfTheMap = 128;

    _graphics->color();
    _graphics->clip();
    _graphics->pal();
}

int Vm::getFps(){
    //TODO: return actual fps (as fix32?)
    return _targetFps;
}

int Vm::getTargetFps(){
    return _targetFps;
}

int Vm::getYear(){
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    return now->tm_year + 1900;
}

int Vm::getMonth(){
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    return now->tm_mon + 1;
}

int Vm::getDay(){
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    return now->tm_mday;
}

int Vm::getHour(){
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    return now->tm_hour;
}

int Vm::getMinute(){
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    return now->tm_min;
}

int Vm::getSecond(){
    std::time_t t = std::time(0);
    std::tm* now = std::localtime(&t);

    return now->tm_sec;
}

std::string Vm::getCartBreadcrumb() {
    return _cartBreadcrumb;
}

std::string Vm::getCartParam() {
    return _cartParam;
}


//settings 
int Vm::getSetting(std::string sname) {
    
    return _host->getSetting(sname);
    
}

void Vm::setSetting(std::string sname, int sval) {
    
    _host->setSetting(sname,sval);
    
}


void Vm::installPackins() {
    #if LOAD_PACK_INS
    _host->setSetting("packinloaded",0);
    _host->unpackCarts();
    #endif
}




void Vm::loadLabel(std::string filename, bool mini, int minioffset) {
    
    auto cartDir = _host->getCartDirectory();
    Cart *labelcart = new Cart(filename, cartDir);
    std::string labelstr = labelcart->LabelString;
    if(labelstr.length() == 0){
        labelstr = NoLabelString;
    }
    if (mini) {
        copy_mini_label_to_sprite_memory(_memory->spriteSheetData, labelstr, minioffset);
    } else {
        copy_string_to_sprite_memory(_memory->spriteSheetData, labelstr);
    }
    delete labelcart;
    
}

std::string Vm::getLuaLine(string filename, int linenumber) {
    
    auto cartDir = _host->getCartDirectory();
    Cart *luacart = new Cart(filename, cartDir);
    std::string luastr = luacart->LuaString;
    
    std::string line;
    std::istringstream luastream(luastr);
    
    while (linenumber-- >= 0){
        std::getline(luastream,line);
    }
    
    delete luacart;
    
    return line;
    
}
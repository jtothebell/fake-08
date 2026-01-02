#include "doctest.h"
#include "stubhost.h"
#include "../source/picoluaapi.h"
#include "../source/fontdata.h"
#include "../source/Input.h"
#include "../source/vm.h"

//extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
//}

TEST_CASE("audio stats") {
  // setup
  PicoRam picoRam;
  picoRam.Reset();
  Audio* audio = new Audio(&picoRam);
  std::string fontdata = get_font_data();
  Graphics* graphics = new Graphics(fontdata, &picoRam);
  //audioState_t* audioState = audio->getAudioState();
  Input * input = new Input(&picoRam);
  StubHost* stubHost = new StubHost();
  Vm* vm = new Vm(stubHost, &picoRam, graphics, input, audio);
  initPicoApi(&picoRam, graphics,  input, vm, audio);
  lua_State *L = luaL_newstate();

  SUBCASE("get sfx 0") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 16);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 5);
  }
  SUBCASE("get sfx 1") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 17);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 6);
  }
  SUBCASE("get sfx 2") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 18);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 7);
  }
  SUBCASE("get sfx 3") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 19);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 8);
  }

  SUBCASE("get sfx 0 updated") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 46);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 5);
  }
  SUBCASE("get sfx 1 updated") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 47);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 6);
  }
  SUBCASE("get sfx 2 updated") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 48);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 7);
  }
  SUBCASE("get sfx 3 updated") {
    audio->api_sfx(5,0,0,31);
    audio->api_sfx(6,1,0,31);
    audio->api_sfx(7,2,0,31);
    audio->api_sfx(8,3,0,31);

    lua_pushnumber(L, 49);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 8);
  }

  SUBCASE("get note 0") {
    audio->api_sfx(5,0,0,31);
    lua_pushnumber(L, 20);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 0);
  }

  SUBCASE("get note 0 updated") {
    audio->api_sfx(5,0,0,31);
    lua_pushnumber(L, 20);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 0);
  }

  SUBCASE("get music") {
    audio->api_music(5,0,0);
    lua_pushnumber(L, 24);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 5);

  }

  SUBCASE("get music updated") {
    audio->api_music(5,0,0);
    lua_pushnumber(L, 54);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 5);
  }

  SUBCASE("get music pattern count") {
    audio->api_music(5,0,0);
    lua_pushnumber(L, 25);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 0);
  }

  SUBCASE("get music pattern count updated") {
    audio->api_music(5,0,0);
    lua_pushnumber(L, 55);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 0);
  }

  SUBCASE("get music tick count") {
    audio->api_music(5,0,0);
    lua_pushnumber(L, 26);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 0);
  }

  SUBCASE("get music tick count updated") {
    audio->api_music(5,0,0);
    lua_pushnumber(L, 56);
    stat(L);
    CHECK_EQ((int) lua_tonumber(L, -1), 0);
  }

}


TEST_CASE("print") {
  // setup
  PicoRam picoRam;
  picoRam.Reset();
  Audio* audio = new Audio(&picoRam);
  std::string fontdata = get_font_data();
  Graphics* graphics = new Graphics(fontdata, &picoRam);
  //audioState_t* audioState = audio->getAudioState();
  Input * input = new Input(&picoRam);
  StubHost* stubHost = new StubHost();
  Vm* vm = new Vm(stubHost, &picoRam, graphics, input, audio);
  initPicoApi(&picoRam, graphics,  input, vm, audio);
  lua_State *L = luaL_newstate();

  SUBCASE("print puts a newline at the end implicitly") {
    lua_pushstring(L, "hello world");
    print(L);
    
    CHECK_EQ(picoRam.drawState.text_x, 0);
    CHECK_EQ(picoRam.drawState.text_y, 6);
  }
  
  SUBCASE("print gets the whole string including nulls") {
    lua_pushlstring(L, "hello world\0", 12);
    print(L);
    
    CHECK_EQ(picoRam.drawState.text_x, 11*4);
    CHECK_EQ(picoRam.drawState.text_y, 0);
  }
}


TEST_CASE("peek with large count") {
  // setup
  PicoRam picoRam;
  picoRam.Reset();
  Audio* audio = new Audio(&picoRam);
  std::string fontdata = get_font_data();
  Graphics* graphics = new Graphics(fontdata, &picoRam);
  Input* input = new Input(&picoRam);
  StubHost* stubHost = new StubHost();
  Vm* vm = new Vm(stubHost, &picoRam, graphics, input, audio);
  initPicoApi(&picoRam, graphics, input, vm, audio);
  lua_State *L = luaL_newstate();

  SUBCASE("peek with 1000 return values does not crash") {
    // Write known values to memory
    for (int i = 0; i < 1000; i++) {
      picoRam.data[0x1000 + i] = (uint8_t)(i & 0xFF);
    }

    lua_pushnumber(L, 0x1000);  // addr
    lua_pushnumber(L, 1000);     // count
    int numResults = peek(L);

    CHECK_EQ(numResults, 1000);
  }

  SUBCASE("peek with 8192 return values (PICO-8 max) does not crash") {
    // Write known values to memory
    for (int i = 0; i < 8192; i++) {
      picoRam.data[0x1000 + i] = (uint8_t)(i & 0xFF);
    }

    lua_pushnumber(L, 0x1000);  // addr
    lua_pushnumber(L, 8192);    // count (PICO-8 max)
    int numResults = peek(L);

    CHECK_EQ(numResults, 8192);
  }

  delete vm;
  delete audio;
  delete graphics;
  delete input;
  delete stubHost;
}

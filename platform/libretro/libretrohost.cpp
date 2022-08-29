
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "libretrohosthelpers.h"
#include "../../source/host.h"
#include "../../source/hostVmShared.h"
#include "../../source/nibblehelpers.h"



static uint8_t stubCurrKDown;
static uint8_t stubCurrKHeld;
static bool stubCurrKBdown = false;
static std::string stubCurrKBkey = "";
static int16_t stubMouseX;
static int16_t stubMouseY;
static uint8_t stubMouseBtns;

std::string _currentCartDirectory;

void setInputState(uint8_t kDown, uint8_t kHeld, int16_t mouseX, int16_t mouseY, uint8_t mouseBtns) {
    stubCurrKDown = kDown;
    stubCurrKHeld = kHeld;
    stubMouseX = mouseX;
    stubMouseY = mouseY;
    stubMouseBtns = mouseBtns;
}

void setCartDirectory(std::string dir) {
    _currentCartDirectory = dir;
}



Host::Host() { }


void Host::oneTimeSetup(Audio* audio){

}

void Host::oneTimeCleanup(){

}

void Host::setTargetFps(int targetFps){

}

void Host::changeStretch(){

}

void Host::forceStretch(StretchOption newStretch) {
    
}



InputState_t Host::scanInput(){
    return InputState_t {stubCurrKDown, stubCurrKHeld, stubMouseX, stubMouseY, stubMouseBtns, stubCurrKBdown, stubCurrKBkey};
}

bool Host::shouldQuit() {
    return false;
}

void Host::waitForTargetFps(){
    
}


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t screenMode){
    
}

bool Host::shouldFillAudioBuff(){
    return false;
}

void* Host::getAudioBufferPointer(){
    return nullptr;
}

size_t Host::getAudioBufferSize(){
    return 0;
}

void Host::playFilledAudioBuffer(){

}

bool Host::shouldRunMainLoop(){
    if (shouldQuit()){
        return false;
    }

    return true;
}

vector<string> Host::listcarts(){
    vector<string> carts;

    return carts;
}

std::string Host::customBiosLua() {
    return "";
}

std::string Host::getCartDirectory() {
    return _currentCartDirectory;
}

void Host::overrideLogFilePrefix(const char* newPrefix) {
    _logFilePrefix = newPrefix;
    
    struct stat st = {0};

    string cartdatadir = _logFilePrefix + "cdata";
    if (stat(cartdatadir.c_str(), &st) == -1) {
        mkdir(cartdatadir.c_str(), 0777);
    }
}

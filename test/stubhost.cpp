
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fstream>
#include <iostream>
using namespace std;

#include "../source/host.h"
#include "../source/hostVmShared.h"
#include "../source/nibblehelpers.h"
#include "stubhost.h"



static uint8_t stubCurrKDown;
static uint8_t stubCurrKHeld;
static bool stubCurrKBdown = false;
static std::string stubCurrKBkey = "";



Host::Host() { }
StubHost::StubHost() { }


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

void StubHost::stubInput(uint8_t kdown, uint8_t kheld) {
    stubCurrKDown = kdown;
    stubCurrKHeld = kheld;
}

InputState_t Host::scanInput(){
    return InputState_t {stubCurrKDown, stubCurrKHeld, 0, 0, 0, stubCurrKBdown, stubCurrKBkey};
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

std::string Host::getCartDataFile(std::string cartDataKey) {
    return "";
}

std::string Host::getCartDataFileContents(std::string cartDataKey) {
    return "";
}

void Host::saveCartData(std::string cartDataKey, std::string contents) {
}

void Host::writeBufferToFile(std::string fileName, char* buffer, size_t length) {
}

size_t Host::getFileContents(std::string fileName, char* buffer) {
    return 0;
}

int Host::getSetting(std::string sname) {
    return 0;
}

void Host::setSetting(std::string sname, int sval) {
    
}

std::string Host::getCartDirectory() {
    return "carts";
}


void Host::setUpPaletteColors(){
    _paletteColors[0] = COLOR_00;
    _paletteColors[1] = COLOR_01;
    _paletteColors[2] = COLOR_02;
    _paletteColors[3] = COLOR_03;
    _paletteColors[4] = COLOR_04;
    _paletteColors[5] = COLOR_05;
    _paletteColors[6] = COLOR_06;
    _paletteColors[7] = COLOR_07;
    _paletteColors[8] = COLOR_08;
    _paletteColors[9] = COLOR_09;
    _paletteColors[10] = COLOR_10;
    _paletteColors[11] = COLOR_11;
    _paletteColors[12] = COLOR_12;
    _paletteColors[13] = COLOR_13;
    _paletteColors[14] = COLOR_14;
    _paletteColors[15] = COLOR_15;

    for (int i = 16; i < 128; i++) {
        _paletteColors[i] = {0, 0, 0, 0};
    }

    _paletteColors[128] = COLOR_128;
    _paletteColors[129] = COLOR_129;
    _paletteColors[130] = COLOR_130;
    _paletteColors[131] = COLOR_131;
    _paletteColors[132] = COLOR_132;
    _paletteColors[133] = COLOR_133;
    _paletteColors[134] = COLOR_134;
    _paletteColors[135] = COLOR_135;
    _paletteColors[136] = COLOR_136;
    _paletteColors[137] = COLOR_137;
    _paletteColors[138] = COLOR_138;
    _paletteColors[139] = COLOR_139;
    _paletteColors[140] = COLOR_140;
    _paletteColors[141] = COLOR_141;
    _paletteColors[142] = COLOR_142;
    _paletteColors[143] = COLOR_143;
}

Color* Host::GetPaletteColors(){
    return _paletteColors;
}


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


Host::Host() { }
StubHost::StubHost() { }


void Host::oneTimeSetup(Color* paletteColors, Audio* audio){

}

void Host::oneTimeCleanup(){

}

void Host::setTargetFps(int targetFps){

}

void Host::changeStretch(){

}

void StubHost::stubInput(uint8_t kdown, uint8_t kheld) {
    stubCurrKDown = kdown;
    stubCurrKHeld = kheld;
}

InputState_t Host::scanInput(){
    return InputState_t {stubCurrKDown, stubCurrKHeld};
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

std::string Host::getCartDirectory() {
    return "carts";
}

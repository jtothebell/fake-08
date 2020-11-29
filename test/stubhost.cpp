
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



static uint8_t currKDown;
static uint8_t currKHeld;


Host::Host() { }
StubHost::StubHost() { }


void Host::oneTimeSetup(Color* paletteColors){

}

void Host::oneTimeCleanup(){

}

void Host::setTargetFps(int targetFps){

}

void Host::changeStretch(){

}

void StubHost::stubInput(uint8_t kdown, uint8_t kheld) {
    currKDown = kdown;
    currKHeld = kheld;
}

InputState_t Host::scanInput(){
    return InputState_t {currKDown, currKHeld};
}

bool Host::shouldQuit() {
    return false;
}

void Host::waitForTargetFps(){
    
}


void Host::drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap){
    
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

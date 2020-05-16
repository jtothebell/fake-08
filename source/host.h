#pragma once

#include <stdio.h>
#include "hostVmShared.h"

enum StretchOption {
  PixelPerfect,
  StretchToFit,
  StretchAndOverflow
};

class Host {
    public:
    Host();

    void oneTimeSetup();

    void setTargetFps(uint8_t targetFps);

     bool mainLoop();

    void scanInput();
    uint8_t getKeysDown();
    uint8_t getKeysHeld();

    bool shouldQuit();

    void changeStretch();
    
    void waitForTargetFps();

    void drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, Color* paletteColors);

    bool shouldFillAudioBuff();
    void* getAudioBufferPointer();
    size_t getAudioBufferSize();
    void playFilledAudioBuffer();

    void oneTimeCleanup();

    

   
};
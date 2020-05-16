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
    
    int getScreenWidth();
    int getScreenHeight();

    int getSecondScreenWidth();
    int getSecondScreenHeight();

    void setTargetFps(uint8_t targetFps);

    StretchOption getDefaultStretch();
    void changeStretch();

    void scanInput();
    uint8_t getKeysDown();
    uint8_t getKeysHeld();

    bool shouldQuit();

    void gfxSetup();
    void postFlipFunction();
    void gfxCleanup();

    void audioSetup();
    void audioCleanup();
    bool shouldFillAudioBuff();
    void* getAudioBufferPointer();
    size_t getAudioBufferSize();
    void playFilledAudioBuffer();

    void drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, Color* paletteColors);


    void oneTimeSetup();

    void oneTimeCleanup();

    void waitForTargetFps();

    bool mainLoop();
};
#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include "hostVmShared.h"
#include "Audio.h"

enum StretchOption {
  PixelPerfect,
  PixelPerfectStretch,
  StretchToFit,
  StretchToFill,
  StretchAndOverflow
};

class Host {
    uint8_t currKDown;
    uint8_t currKHeld;
    bool lDown = false;
    bool rDown = false;
    bool stretchKeyPressed = false;
    StretchOption stretch = PixelPerfectStretch;
    int quit = 0;


    std::string _logFilePrefix;
    std::string _customBiosLua;

    void loadSettingsIni();
    void saveSettingsIni();


    public:
    Host();

    void oneTimeSetup(Color* paletteColors, Audio* audio);
    
    void setTargetFps(int targetFps);

    bool shouldRunMainLoop();

    InputState_t scanInput();
    bool shouldQuit();

    void changeStretch();
    
    void waitForTargetFps();

    void drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap);

    bool shouldFillAudioBuff();
    void* getAudioBufferPointer();
    size_t getAudioBufferSize();
    void playFilledAudioBuffer();

    void oneTimeCleanup();

    double deltaTMs();

    std::vector<std::string> listcarts();

    const char* logFilePrefix();

    std::string customBiosLua();

    void setPlatformParams(
        int windowWidth,
        int windowHeight,
        uint32_t sdlWindowFlags,
        uint32_t sdlRendererFlags,
        uint32_t sdlPixelFormat,
        std::string logFilePrefix,
        std::string customBiosLua);
};

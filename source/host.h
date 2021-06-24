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
  StretchAndOverflow,
  AltScreenPixelPerfect,
  AltScreenStretch,
  FourByThreeVertPerfect,
  FourByThreeStretch
};

class Host {
    uint8_t currKDown;
    uint8_t currKHeld;
    bool lDown = false;
    bool rDown = false;
    bool stretchKeyPressed = false;
    StretchOption stretch = PixelPerfectStretch;
    float scaleX = 1.0;
    float scaleY = 1.0;
    int mouseOffsetX = 0;
    int mouseOffsetY = 0;
    int quit = 0;


    std::string _logFilePrefix;
    std::string _customBiosLua;
    std::string _cartDirectory;

    void loadSettingsIni();
    void saveSettingsIni();

    std::string getCartDataFile(std::string cartDataKey);

    public:
    Host();

    void oneTimeSetup(Color* paletteColors, Audio* audio);
    
    void setTargetFps(int targetFps);

    bool shouldRunMainLoop();

    InputState_t scanInput();
    bool shouldQuit();

    void changeStretch();
    
    void waitForTargetFps();

    void drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t drawMode);

    bool shouldFillAudioBuff();
    void* getAudioBufferPointer();
    size_t getAudioBufferSize();
    void playFilledAudioBuffer();

    void oneTimeCleanup();

    double deltaTMs();

    std::vector<std::string> listcarts();

    const char* logFilePrefix();

    std::string customBiosLua();

    std::string getCartDataFileContents(std::string cartDataKey);

    void saveCartData(std::string cartDataKey, std::string contents);

    std::string getCartDirectory();

    void setPlatformParams(
        int windowWidth,
        int windowHeight,
        uint32_t sdlWindowFlags,
        uint32_t sdlRendererFlags,
        uint32_t sdlPixelFormat,
        std::string logFilePrefix,
        std::string customBiosLua,
        std::string cartDirectory);
};

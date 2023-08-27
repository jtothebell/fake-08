#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include "hostVmShared.h"
#include "Audio.h"

#if (defined(_WIN32) || defined(__WIN32__))
#include <direct.h> /* _mkdir */
#define mkdir(A, B) _mkdir(A)
#endif


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


#if LOAD_PACK_INS
enum PackinLoadOption {
  Unloaded,
  Loaded
};
#endif

enum ResizekeyOption {
  NoResize,
  YesResize
};

enum KeyboardOption {
  Emoji,
  Lowercase
};

enum MenuStyleOption {
  Classic,
  Fancy,
  Splore
};

enum BgColorOption {
  Gray,
  Black,
  Blue,
  Green,
  Purple,
  White
};

class Host {
    uint8_t currKDown;
    uint8_t currKHeld;
	
    bool currKBDown = false;
    std::string currKBKey = "";
	
    bool lDown = false;
    bool rDown = false;
    bool stretchKeyPressed = false;
	
	
	//settings
	#if LOAD_PACK_INS
    PackinLoadOption packinloaded = Unloaded;
	#endif
    StretchOption stretch = PixelPerfectStretch;
    KeyboardOption kbmode = Emoji;
    ResizekeyOption resizekey = NoResize;
    MenuStyleOption menustyle = Fancy;
    BgColorOption bgcolor = Gray;
	
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

    Color _paletteColors[144];

    public:
    Host();

    void setUpPaletteColors();
    void oneTimeSetup(Audio* audio);
	
    void unpackCarts();
    
    void setTargetFps(int targetFps);

    bool shouldRunMainLoop();

    InputState_t scanInput();
    bool shouldQuit();

    void changeStretch();
    void forceStretch(StretchOption newStretch);
    
    void waitForTargetFps();

    void drawFrame(uint8_t* picoFb, uint8_t* screenPaletteMap, uint8_t drawMode);

    bool shouldFillAudioBuff();
    void* getAudioBufferPointer();
    size_t getAudioBufferSize();
    void playFilledAudioBuffer();

    void oneTimeCleanup();

    double deltaTMs();

    std::vector<std::string> listcarts();

    void overrideLogFilePrefix(const char* newPrefix);
    const char* logFilePrefix();

    std::string customBiosLua();

    std::string getCartDataFileContents(std::string cartDataKey);

    void saveCartData(std::string cartDataKey, std::string contents);

    size_t getFileContents(std::string fileName, char* buffer);
    void writeBufferToFile(std::string cartDataKey, char* buffer, size_t length);

    std::string getCartDirectory();
	
    //settings
    int getSetting(std::string sname);
    void setSetting(std::string sname, int sdata);

    Color* GetPaletteColors();

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

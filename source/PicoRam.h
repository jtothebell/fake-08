#pragma once

#include <cstring> 
#include <string>

/*
0x0 	0x0fff 	Sprite sheet (0-127)
0x1000 	0x1fff 	Sprite sheet (128-255) / Map (rows 32-63) (shared)
0x2000 	0x2fff 	Map (rows 0-31)
0x3000 	0x30ff 	Sprite flags
0x3100 	0x31ff 	Music
0x3200 	0x42ff 	Sound effects
0x4300 	0x5dff 	General use (or work RAM)
0x5e00 	0x5eff 	Persistent cart data (64 numbers = 256 bytes)
0x5f00 	0x5f3f 	Draw state
0x5f40 	0x5f7f 	Hardware state
0x5f80 	0x5fff 	GPIO pins (128 bytes)
0x6000 	0x7fff 	Screen data (8k) 
*/


//used to use a bitfield union for song and sfx. but this caused problems on big endian platforms
//now use specific getter/setters that make sure bits are in the right spot
struct song {
    // The four song channels that should play, 0…63 (each msb holds a flag)
    uint8_t data[4];

    uint8_t getSfx0()const {
        return (data[0] & 0b01111111);
    }
    uint8_t getSfx1()const {
        return (data[1] & 0b01111111);
    }
    uint8_t getSfx2()const {
        return (data[2] & 0b01111111);
    }
    uint8_t getSfx3()const {
        return (data[3] & 0b01111111);
    }

    uint8_t getStart()const {
        return (data[0] & 0b10000000) >> 7;
    }
    uint8_t getLoop()const {
        return (data[1] & 0b10000000) >> 7;
    }
    uint8_t getStop()const {
        return (data[2] & 0b10000000) >> 7;
    }
    uint8_t getMode()const {
        return (data[3] & 0b10000000) >> 7;
    }
};

struct note {
    uint8_t data[2];

    void setKey(uint8_t val){
        uint8_t mask = 0b00111111;
        data[0] = (data[0] & ~mask) | (val & mask);
    }
    void setWaveform(uint8_t val){
        //waveform spans both bytes
        uint8_t val0 = val << 6;
        uint8_t mask0 = 0b11000000;
        data[0] = (data[0] & ~mask0) | (val0 & mask0);

        uint8_t val1 = val >> 2;
        uint8_t mask1 = 0b00000001;
        data[1] = (data[1] & ~mask1) | (val1 & mask1);
    }
    void setVolume(uint8_t val){
        uint8_t mask = 0b00001110;
        data[1] = (data[1] & ~mask) | ((val << 1) & mask);
    }
    void setEffect(uint8_t val){
        uint8_t mask = 0b01110000;
        data[1] = (data[1] & ~mask) | ((val << 4) & mask);
    }
    void setCustom(uint8_t val){
        uint8_t mask = 0b10000000;
        data[1] = (data[1] & ~mask) | ((val << 7) & mask);
    }


    uint8_t getKey()const {
        return (data[0] & 0b00111111);
    }
    uint8_t getWaveform()const {
        //waveform spans both bytes
        return ((data[1] & 0b00000001) << 2) + ((data[0] & 0b11000000) >> 6);
    }
    uint8_t getVolume()const {
        return ((data[1] & 0b00001110) >> 1);
    }
    uint8_t getEffect()const {
        return ((data[1] & 0b01110000) >> 4);
    }
    uint8_t getCustom()const {
        return ((data[1] & 0b10000000) >> 7);
    }
};

struct sfx {
    union
    {
        struct 
        {
            note notes[32];

            uint8_t editorMode;
            uint8_t speed;
            uint8_t loopRangeStart;
            uint8_t loopRangeEnd;
        };

        uint8_t data[68];
    };
};

struct musicChannel {
    int16_t count = 0;
    int16_t pattern = -1;
    int8_t master = -1;
    uint8_t mask = 0xf;
    uint8_t speed = 0;
    float volume = 0.f;
    float volume_step = 0.f;
    float offset = 0.f;
	uint8_t length = 0;
};

struct noteChannel {
    float phi = 0;
    note n;
};

struct rawSfxChannel {
    int16_t sfxId = -1;
    float offset = 0;
    bool can_loop = true;
    bool is_music = false;
    noteChannel current_note;
    noteChannel prev_note;
    virtual rawSfxChannel *getChildChannel() {
      return NULL;
    }
    virtual rawSfxChannel *getPrevChildChannel() {
      return NULL;
    }
    virtual void rotateChannels() {
    }
};

struct sfxChannel : rawSfxChannel {
    rawSfxChannel customInstrumentChannel;
    rawSfxChannel prevInstrumentChannel;
    virtual rawSfxChannel *getChildChannel() {
      return &(this->customInstrumentChannel);
    }
    virtual rawSfxChannel *getPrevChildChannel() {
      return &(this->prevInstrumentChannel);
    }
    virtual void rotateChannels() {
      prevInstrumentChannel = customInstrumentChannel;
    }
};

struct audioState_t {
    musicChannel _musicChannel;
    sfxChannel _sfxChannels[4];
};

struct drawState_t {
    uint8_t drawPaletteMap[16];
	uint8_t screenPaletteMap[16];

    uint8_t clip_xb;
	uint8_t clip_yb;
	uint8_t clip_xe;
	uint8_t clip_ye;

    uint8_t unknown05f24;

    uint8_t color;

    uint8_t text_x;
	uint8_t text_y;

    int16_t camera_x;
	int16_t camera_y;

    uint8_t drawMode;

    uint8_t devkitMode;

    uint8_t persistPalette;

    uint8_t soundPauseState;

    uint8_t suppressPause;

    uint8_t fillPattern[2];

    uint8_t fillPatternTransparencyBit;

    uint8_t colorSettingFlag;

    uint8_t lineInvalid;

    //hardware extension
    uint8_t unknown05f36;
    uint8_t unknown05f37;

    uint8_t tlineMapWidth;
    uint8_t tlineMapHeight;
    uint8_t tlineMapXOffset;
    uint8_t tlineMapYOffset;

    int16_t line_x;
    int16_t line_y;
};


struct hwState_t {
    //audio hardware mods
    //0x5f40
    uint8_t half_rate;
    //0x5f41 
    uint8_t reverb;
    //0x5f42
    uint8_t distort;
    //0x5f43
    uint8_t lowpass;
    //0x5f44..0x5f4b
    uint32_t rngState[2];
    //0x5f4c..0x5f53
    uint8_t buttonStates[8];
    //0x5f54
    uint8_t spriteSheetMemMapping;
    //0x5f55
    uint8_t screenDataMemMapping;
    //0x5f56
    uint8_t mapMemMapping;
    //0x5f57
    uint8_t widthOfTheMap;
    //0x5f58
    uint8_t printAttributes;
    //0x5f59
    uint8_t printCharDimensions;
    //0x5f5a
    uint8_t printTabWidth;
    //0x5f5b
    uint8_t printOffsetDimensions;
    //0x5f5c
    uint8_t btnpRepeatDelay;
    //0x5f5d
    uint8_t btnpRepeatInterval;
    //0x5f5e
    uint8_t colorBitmask;
    //0x5f5f
    uint8_t alternatePaletteFlag;
    //0x5f60..0x5f6f
    uint8_t alternatePaletteMap[16];
    //0x5f70..0x5f7f
    uint8_t alternatePaletteScreenLineBitfield[16];
    //0x5f80..0x5fff
    uint8_t gpioPins[128];

};



struct PicoRam
{
    void Reset() {
        memset(data, 0, 0x4300);
        //leave general use memory
        memset(data + 0x5f00, 0, 0x8000 - 0x5f00);
        //colorBitmask starts at 255
        hwState.colorBitmask = 0xff;
        hwState.spriteSheetMemMapping = 0x00;
        hwState.screenDataMemMapping = 0x60;
        hwState.mapMemMapping = 0x20;
        hwState.widthOfTheMap = 128;
    }
    
    union
    {
        struct 
        {
            //0x0     0x0fff    Sprite sheet (0-127)
            //0x1000  0x1fff    Sprite sheet (128-255) / Map (rows 32-63) (shared) 
            uint8_t spriteSheetData[128 * 64];
            //0x2000  0x2fff    Map (rows 0-31) 
            uint8_t mapData[128 * 32];
            //0x3000  0x30ff    Sprite flags 
            uint8_t spriteFlags[256];
            //0x3100  0x31ff    Music
            struct song songs[64];
            //0x3200  0x42ff    Sound effects
            struct sfx sfx[64];
            //0x4300  0x5dff    General use (or work RAM)
            uint8_t generalUseRam[6912];
            //0x5e00  0x5eff    Persistent cart data (64 numbers = 256 bytes)
            uint8_t cartData[256];
            
            drawState_t drawState;

            hwState_t hwState;

            uint8_t screenBuffer[128 * 64];

            uint8_t userData[0x8000];
        };

        uint8_t data[0x10000];
    };
};

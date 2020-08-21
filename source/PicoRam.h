#pragma once

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

struct song {
    union
    {
        struct
        {
            uint8_t sfx0  : 7;
            uint8_t start : 1;
            uint8_t sfx1  : 7;
            uint8_t loop  : 1;
            uint8_t sfx2  : 7;
            uint8_t stop  : 1;
            uint8_t sfx3  : 7;
            uint8_t mode  : 1;
        };

        // The four song channels that should play, 0…63 (each msb holds a flag)
        uint8_t data[4];
    };
};

//using uint16_t may be necessary since waveform spans two bytes
struct note {
    union
    {
        struct
        {
            uint16_t key : 6;
            uint16_t waveform : 3;
            uint16_t volume : 3;
            uint16_t effect : 4;
        };
        
        uint8_t data[2];
    };
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
};

struct sfxChannel {
    int16_t sfxId = -1;
    float offset = 0;
    float phi = 0;
    bool can_loop = true;
    bool is_music = false;
    int8_t prev_key = 0;
    float prev_vol = 0;
};

struct audioState {
    musicChannel _musicChannel;
    sfxChannel _sfxChannels[4];
};

struct drawState {
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

    uint8_t camera_x;
	uint8_t camera_y;

    uint8_t drawMode;

    uint8_t devkitMode;

    uint8_t persistPalette;

    uint8_t suppressPause;

    uint16_t fillPattern;

    uint8_t fillPatternTransparencyBit;

    uint8_t colorSettingFlag;

    uint8_t lineInvalid;

    uint8_t unknown05f36;
    uint8_t unknown05f37;

    uint8_t tlineMapWidth;
    uint8_t tlineMapHeight;
    uint8_t tlineMapXOffset;
    uint8_t tlineMapYOffset;

    int16_t line_x;
    int16_t line_y;
};


struct hwState {
    uint8_t audioHardwareState[4];

    uint8_t rngState[8];

    uint8_t buttonStates[8];

    uint8_t unknownInputBlock[8];

    uint8_t btnpRepeatDelay;

    uint8_t btnpRepeatInterval;

    uint8_t colorBitmask;

    uint8_t alternatePaletteFlag;

    uint8_t alternatePaletteMap[16];

    uint8_t alternatePaletteScreenLineBitfield[16];

    uint8_t gpioPins[128];

};



struct PicoRam
{
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
            musicChannel _musicChannel;
            sfxChannel _sfxChannels[4];
            
            drawState drawState;

            hwState hwState;

            //TODO use draw state struct here
            /*
            uint8_t _gfxState_color;
            uint8_t _gfxState_bgColor;

            int _gfxState_text_x;
            int _gfxState_text_y;

            int _gfxState_camera_x;
            int _gfxState_camera_y;

            int _gfxState_clip_xb;
            int _gfxState_clip_yb;
            int _gfxState_clip_xe;
            int _gfxState_clip_ye;

            uint8_t _gfxState_drawPaletteMap[16];
            uint8_t _gfxState_screenPaletteMap[16];
            bool _gfxState_transparencyPalette[16];

            //not actually part of graphics state memory?
            int _gfxState_line_x;
            int _gfxState_line_y;
            bool _gfxState_line_valid;
            */

            

            uint8_t screenBuffer[128 * 64];
        };

        uint8_t data[0x8000];
    };
};
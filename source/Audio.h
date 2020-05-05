#pragma once

#include <string>

#define MAX_SFX = 64
#define BYTES_PER_SFX = 68;

#define MAX_MUSIC = 64
#define BYTES_PER_MUSIC = 4;

//best info about sfx/music and differences between cart and memory that I've found
//https://github.com/dansanderson/picotool:
/*

MUSIC:
The music region consists of 256 bytes. The .p8 representation is one
line for each of 64 patterns, with a hex-encoded flags byte, a space,
and four hex-encoded one-byte sound numbers.
The flags are:
 1: begin pattern loop
 2: end pattern loop
 4: stop at end of pattern
The sound numbers represents four channels played simultaneously up to
the shortest pattern. The sounds are numbered 0 through 63
(0x00-0x3f). If a channel is silent for a song pattern, its number is
64 + the channel number (0x41, 0x42, 0x43, or 0x44).
The in-memory (and PNG) representation is slightly different from the
.p8 representation. Instead of storing the flags in a separate byte,
the flags are stored in the highest bit of the first three channels,
for a total of four bytes:
 chan1 & 128: stop at end of pattern
 chan2 & 128: end pattern loop
 chan3 & 128: begin pattern loop

SFX:
 The sound effects region consists of 4352 bytes. The .p8
representation is 64 lines of 168 hexadecimal digits (84 bytes).
Each line represents one sound effect/music pattern. The values are as follows:
 0    The editor mode: 0 for pitch mode, 1 for note entry mode.
 1    The note duration, in multiples of 1/128 second.
 2    Loop range start, as a note number (0-63).
 3    Loop range end, as a note number (0-63).
4-84  32 notes:
        0: pitch (0-63): c-0 to d#-5, chromatic scale
        1-high: waveform (0-F):
          0 sine, 1 triangle, 2 sawtooth, 3 long square, 4 short square,
          5 ringing, 6 noise, 7 ringing sine; 8-F are the custom instruments
          corresponding to sfx 0-7
        1-low: volume (0-7)
        2-high: effect (0-7):
          0 none, 1 slide, 2 vibrato, 3 drop, 4 fade_in, 5 fade_out,
          6 arp fast, 7 arp slow; arpeggio commands loop over groups of
          four notes at speed 2 (fast) and 4 (slow)
      One note uses five nibbles, so two notes use five bytes.
The RAM representation is different. Each pattern is 68 bytes, with
two bytes for each of 32 notes, one byte for the editor mode, one byte
for the speed, and two bytes for the loop range (start, end). Each
note is encoded in 16 bits, LSB first, like so:
  w2-w1-pppppp c-eee-vvv-w3
  eee: effect (0-7)
  vvv: volume (0-7)
  w3w2w1: waveform (0-7)
  pppppp: pitch (0-63) 
  c: if 1, waveform is a custom instrument corresponding to sfx 0-7;
    otherwise it's one of the eight built-in waveforms
(Considering waveform as a value from 0-15, c is w4.)

*/
//todo: make these memory compatible later, but for now, this representation
//is easier for me to wrap my mind araound

//this is also defined in audio? should probably consolidate
#define BITMASK(n) (1U<<(n))

struct song {
    uint8_t loop;
 
    uint8_t channel1;
    uint8_t channel2;
    uint8_t channel3;
    uint8_t channel4;

};

struct note {
    uint8_t key;
    uint8_t waveform;
    uint8_t volume;
    uint8_t effect;
};

struct sfx {
    note notes[32];

    uint8_t editorMode;
    uint8_t speed;
    uint8_t loopRangeStart;
    uint8_t loopRangeEnd;
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



class Audio {
    //uint8_t _musicRam[64 * 4];
    song _songs[64];

    //uint8_t _sfxRam[64 * 68];
    sfx _sfx[64];

    musicChannel _musicChannel;
    sfxChannel _sfxChannels[4];

    int16_t getSampleForChannel(int channel);

    void set_music_pattern(int pattern);
    
    public:
    Audio();

    void setSfx(std::string sfxString);
    void setMusic(std::string musicString);

    void api_sfx(uint8_t sfx, int channel, int offset);
    void api_music(uint8_t pattern, int16_t fade_len, int16_t mask);

    void FillAudioBuffer(void *audioBuffer,size_t offset, size_t size);
};


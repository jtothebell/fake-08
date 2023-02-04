#pragma once

#include "PicoRam.h"

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

enum
{
    FX_NO_EFFECT = 0,
    FX_SLIDE =     1,
    FX_VIBRATO =   2,
    FX_DROP =      3,
    FX_FADE_IN =   4,
    FX_FADE_OUT =  5,
    FX_ARP_FAST =  6,
    FX_ARP_SLOW =  7,
};


class Audio {
    PicoRam* _memory;
    audioState_t _audioState;

    void set_music_pattern(int pattern);
    
    public:
    float getSampleForSfx(rawSfxChannel &channel, float freqShift = 1.0f);
    int16_t getSampleForChannel(int channel);
    float getSampleForNote(noteChannel &note_channel, rawSfxChannel &parentChannel, rawSfxChannel *childChannel, note prev_note, float freqShift, bool forceRemainder);

    public:
    Audio(PicoRam* memory);

    void resetAudioState();
    audioState_t* getAudioState();

    void api_sfx(int sfx, int channel, int offset);
    void api_music(int pattern, int16_t fade_len, int16_t mask);

    int16_t getCurrentSfxId(int channel);
    int getCurrentNoteNumber(int channel);
    int16_t getCurrentMusic();
    int16_t getMusicPatternCount();
    int16_t getMusicTickCount();

    void FillAudioBuffer(void *audioBuffer,size_t offset, size_t size);
    void FillMonoAudioBuffer(void *audioBuffer,size_t offset, size_t size);
};


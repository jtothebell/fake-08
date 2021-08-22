#include "Audio.h"
#include "synth.h"
#include "hostVmShared.h"
#include "mathhelpers.h"

#include <string>
#include <sstream>
#include <algorithm> // std::max
#include <cmath>
#include <float.h> // std::max

//playback implemenation based on zetpo 8's
//https://github.com/samhocevar/zepto8/blob/master/src/pico8/sfx.cpp

Audio::Audio(PicoRam* memory){
    _memory = memory;
    
    resetAudioState();
}

void Audio::resetAudioState() {
    for(int i = 0; i < 4; i++) {
        _audioState._sfxChannels[i].sfxId = -1;
        _audioState._sfxChannels[i].offset = 0;
        _audioState._sfxChannels[i].phi = 0;
        _audioState._sfxChannels[i].can_loop = true;
        _audioState._sfxChannels[i].is_music = false;
        _audioState._sfxChannels[i].prev_key = 0;
        _audioState._sfxChannels[i].prev_vol = 0;
    }
    _audioState._musicChannel.count = 0;
    _audioState._musicChannel.pattern = -1;
    _audioState._musicChannel.master = -1;
    _audioState._musicChannel.mask = 0xf;
    _audioState._musicChannel.speed = 0;
    _audioState._musicChannel.volume = 0.f;
    _audioState._musicChannel.volume_step = 0.f;
    _audioState._musicChannel.offset = 0.f;
}

audioState* Audio::getAudioState() {
    return &_audioState;
}

void Audio::api_sfx(int sfx, int channel, int offset){

    if (sfx < -2 || sfx > 63 || channel < -1 || channel > 3 || offset > 31) {
        return;
    }

    if (sfx == -1)
    {
        // Stop playing the current channel
        if (channel != -1) {
            _audioState._sfxChannels[channel].sfxId = -1;
        }
    }
    else if (sfx == -2)
    {
        // Stop looping the current channel
        if (channel != -1) {
            _audioState._sfxChannels[channel].can_loop = false;
        }
    }
    else
    {
        // Find the first available channel: either a channel that plays
        // nothing, or a channel that is already playing this sample (in
        // this case PICO-8 decides to forcibly reuse that channel, which
        // is reasonable)
        if (channel == -1)
        {
            for (int i = 0; i < 4; ++i)
                if (_audioState._sfxChannels[i].sfxId == -1 ||
                    _audioState._sfxChannels[i].sfxId == sfx)
                {
                    channel = i;
                    break;
                }
        }

        // If still no channel found, the PICO-8 strategy seems to be to
        // stop the sample with the lowest ID currently playing
        if (channel == -1)
        {
            for (int i = 0; i < 4; ++i) {
               if (channel == -1 || _audioState._sfxChannels[i].sfxId < _audioState._sfxChannels[channel].sfxId) {
                   channel = i;
               }
            }
        }

        // Stop any channel playing the same sfx
        for (int i = 0; i < 4; ++i) {
            if (_audioState._sfxChannels[i].sfxId == sfx) {
                _audioState._sfxChannels[i].sfxId = -1;
            }
        }

        // Play this sound!
        _audioState._sfxChannels[channel].sfxId = sfx;
        _audioState._sfxChannels[channel].offset = std::max(0.f, (float)offset);
        _audioState._sfxChannels[channel].phi = 0.f;
        _audioState._sfxChannels[channel].can_loop = true;
        _audioState._sfxChannels[channel].is_music = false;
        // Playing an instrument starting with the note C-2 and the
        // slide effect causes no noticeable pitch variation in PICO-8,
        // so I assume this is the default value for “previous key”.
        _audioState._sfxChannels[channel].prev_key = 24;
        // There is no default value for “previous volume”.
        _audioState._sfxChannels[channel].prev_vol = 0.f;
    }      
}

void Audio::api_music(int pattern, int16_t fade_len, int16_t mask){
    if (pattern < -1 || pattern > 63) {
        return;
    }

    if (pattern == -1)
    {
        // Music will stop when fade out is finished
        _audioState._musicChannel.volume_step = fade_len <= 0 ? -FLT_MAX
                                  : -_audioState._musicChannel.volume * (1000.f / fade_len);
        return;
    }

    _audioState._musicChannel.count = 0;
    _audioState._musicChannel.mask = mask ? mask & 0xf : 0xf;

    _audioState._musicChannel.volume = 1.f;
    _audioState._musicChannel.volume_step = 0.f;
    if (fade_len > 0)
    {
        _audioState._musicChannel.volume = 0.f;
        _audioState._musicChannel.volume_step = 1000.f / fade_len;
    }

    set_music_pattern(pattern);
}

void Audio::set_music_pattern(int pattern) {
    _audioState._musicChannel.pattern = pattern;
    _audioState._musicChannel.offset = 0;

    //array to access song's channels. may be better to have this part of the struct?
    uint8_t channels[] = {
        _memory->songs[pattern].getSfx0(),
        _memory->songs[pattern].getSfx1(),
        _memory->songs[pattern].getSfx2(),
        _memory->songs[pattern].getSfx3(),
    };

    // Find music speed; it’s the speed of the fastest sfx
    _audioState._musicChannel.master = _audioState._musicChannel.speed = -1;
    for (int i = 0; i < 4; ++i)
    {
        uint8_t n = channels[i];

        if (n & 0x40)
            continue;

        auto &sfx = _memory->sfx[n & 0x3f];
        if (_audioState._musicChannel.master == -1 || _audioState._musicChannel.speed > sfx.speed)
        {
            _audioState._musicChannel.master = i;
            _audioState._musicChannel.speed = std::max(1, (int)sfx.speed);
        }
    }

    // Play music sfx on active channels
    for (int i = 0; i < 4; ++i)
    {
        if (((1 << i) & _audioState._musicChannel.mask) == 0)
            continue;

        uint8_t n = channels[i];
        if (n & 0x40)
            continue;

        _audioState._sfxChannels[i].sfxId = n;
        _audioState._sfxChannels[i].offset = 0.f;
        _audioState._sfxChannels[i].phi = 0.f;
        _audioState._sfxChannels[i].can_loop = false;
        _audioState._sfxChannels[i].is_music = true;
        _audioState._sfxChannels[i].prev_key = 24;
        _audioState._sfxChannels[i].prev_vol = 0.f;
    }
}

void Audio::FillAudioBuffer(void *audioBuffer, size_t offset, size_t size){
    if (audioBuffer == nullptr) {
        return;
    }

    uint32_t *buffer = (uint32_t *)audioBuffer;

    for (size_t i = 0; i < size; ++i){
        int16_t sample = 0;

        for (int c = 0; c < 4; ++c) {
            sample += this->getSampleForChannel(c);
        }

        //buffer is stereo, so just send the mono sample to both channels
        buffer[i] = (sample<<16) | (sample & 0xffff);
    }
}

void Audio::FillMonoAudioBuffer(void *audioBuffer, size_t offset, size_t size){
    if (audioBuffer == nullptr) {
        return;
    }

    int16_t *buffer = (int16_t *)audioBuffer;

    for (size_t i = 0; i < size; ++i){
        int16_t sample = 0;

        for (int c = 0; c < 4; ++c) {
            sample += this->getSampleForChannel(c);
        }

        buffer[i] = sample;
    }
}

static float key_to_freq(float key)
{
    using std::exp2;
    return 440.f * exp2((key - 33.f) / 12.f);
}

int16_t Audio::getCurrentSfxId(int channel){
    return _audioState._sfxChannels[channel].sfxId;
}

int Audio::getCurrentNoteNumber(int channel){
    return _audioState._sfxChannels[channel].sfxId < 0 
        ? -1
        : _audioState._sfxChannels[channel].offset;
}

int16_t Audio::getCurrentMusic(){
    return _audioState._musicChannel.pattern;
}

int16_t Audio::getMusicPatternCount(){
    return _audioState._musicChannel.count;
}

int16_t Audio::getMusicTickCount(){
    return _audioState._musicChannel.offset * _audioState._musicChannel.speed;
}

//adapted from zepto8 sfx.cpp (wtfpl license)
int16_t Audio::getSampleForChannel(int channel){
    using std::fabs;
    using std::fmod;
    using std::floor;
    using std::max;

    int const samples_per_second = 22050;

    int16_t sample = 0;

    const int index = _audioState._sfxChannels[channel].sfxId;
 
    // Advance music using the master channel
    if (channel == _audioState._musicChannel.master && _audioState._musicChannel.pattern != -1)
    {
        float const offset_per_second = 22050.f / (183.f * _audioState._musicChannel.speed);
        float const offset_per_sample = offset_per_second / samples_per_second;
        _audioState._musicChannel.offset += offset_per_sample;
        _audioState._musicChannel.volume += _audioState._musicChannel.volume_step / samples_per_second;
        _audioState._musicChannel.volume = clamp(_audioState._musicChannel.volume, 0.f, 1.f);

        if (_audioState._musicChannel.volume_step < 0 && _audioState._musicChannel.volume <= 0)
        {
            // Fade out is finished, stop playing the current song
            for (int i = 0; i < 4; ++i) {
                if (_audioState._sfxChannels[i].is_music) {
                    _audioState._sfxChannels[i].sfxId = -1;
                }
            }
            _audioState._musicChannel.pattern = -1;
        }
        else if (_audioState._musicChannel.offset >= 32.f)
        {
            int16_t next_pattern = _audioState._musicChannel.pattern + 1;
            int16_t next_count = _audioState._musicChannel.count + 1;
            //todo: pull out these flags, get memory storage correct as well
            if (_memory->songs[_audioState._musicChannel.pattern].getStop()) //stop part of the loop flag
            {
                next_pattern = -1;
                next_count = _audioState._musicChannel.count;
            }
            else if (_memory->songs[_audioState._musicChannel.pattern].getLoop()){
                while (--next_pattern > 0 && !_memory->songs[next_pattern].getStart())
                    ;
            }

            _audioState._musicChannel.count = next_count;
            set_music_pattern(next_pattern);
        }
    }

    if (index < 0 || index > 63) {
        //no (valid) sfx here. return silence
        return 0;
    }

    struct sfx const &sfx = _memory->sfx[index];

    // Speed must be 1—255 otherwise the SFX is invalid
    int const speed = max(1, (int)sfx.speed);

    float const offset = _audioState._sfxChannels[channel].offset;
    float const phi = _audioState._sfxChannels[channel].phi;

    // PICO-8 exports instruments as 22050 Hz WAV files with 183 samples
    // per speed unit per note, so this is how much we should advance
    float const offset_per_second = 22050.f / (183.f * speed);
    float const offset_per_sample = offset_per_second / samples_per_second;
    float next_offset = offset + offset_per_sample;

    // Handle SFX loops. From the documentation: “Looping is turned
    // off when the start index >= end index”.
    float const loop_range = float(sfx.loopRangeEnd - sfx.loopRangeStart);
    if (loop_range > 0.f && next_offset >= sfx.loopRangeStart && _audioState._sfxChannels[channel].can_loop) {
        next_offset = fmod(next_offset - sfx.loopRangeStart, loop_range)
                    + sfx.loopRangeStart;
    }

    int const note_idx = (int)floor(offset);
    int const next_note_idx = (int)floor(next_offset);

    uint8_t key = sfx.notes[note_idx].getKey();
    float volume = sfx.notes[note_idx].getVolume() / 7.f;
    float freq = key_to_freq(key);

    if (volume == 0.f){
        //volume all the way off. return silence, but make sure to set stuff
        _audioState._sfxChannels[channel].offset = next_offset;

        if (next_offset >= 32.f){
            _audioState._sfxChannels[channel].sfxId = -1;
        }
        else if (next_note_idx != note_idx){
            _audioState._sfxChannels[channel].prev_key = sfx.notes[note_idx].getKey();
            _audioState._sfxChannels[channel].prev_vol = sfx.notes[note_idx].getVolume() / 7.f;
        }

        return 0;
    }
    
    //TODO: apply effects
    int const fx = sfx.notes[note_idx].getEffect();

    // Apply effect, if any
    switch (fx)
    {
        case FX_NO_EFFECT:
            break;
        case FX_SLIDE:
        {
            float t = fmod(offset, 1.f);
            // From the documentation: “Slide to the next note and volume”,
            // but it’s actually _from_ the _prev_ note and volume.
            freq = lerp(key_to_freq(_audioState._sfxChannels[channel].prev_key), freq, t);
            if (_audioState._sfxChannels[channel].prev_vol > 0.f)
                volume = lerp(_audioState._sfxChannels[channel].prev_vol, volume, t);
            break;
        }
        case FX_VIBRATO:
        {
            // 7.5f and 0.25f were found empirically by matching
            // frequency graphs of PICO-8 instruments.
            float t = fabs(fmod(7.5f * offset / offset_per_second, 1.0f) - 0.5f) - 0.25f;
            // Vibrato half a semi-tone, so multiply by pow(2,1/12)
            freq = lerp(freq, freq * 1.059463094359f, t);
            break;
        }
        case FX_DROP:
            freq *= 1.f - fmod(offset, 1.f);
            break;
        case FX_FADE_IN:
            volume *= fmod(offset, 1.f);
            break;
        case FX_FADE_OUT:
            volume *= 1.f - fmod(offset, 1.f);
            break;
        case FX_ARP_FAST:
        case FX_ARP_SLOW:
        {
            // From the documentation:
            // “6 arpeggio fast  //  Iterate over groups of 4 notes at speed of 4
            //  7 arpeggio slow  //  Iterate over groups of 4 notes at speed of 8”
            // “If the SFX speed is <= 8, arpeggio speeds are halved to 2, 4”
            int const m = (speed <= 8 ? 32 : 16) / (fx == FX_ARP_FAST ? 4 : 8);
            int const n = (int)(m * 7.5f * offset / offset_per_second);
            int const arp_note = (note_idx & ~3) | (n & 3);
            freq = key_to_freq(sfx.notes[arp_note].getKey());
            break;
        }
    }


    // Play note
    float waveform = z8::synth::waveform(sfx.notes[note_idx].getWaveform(), phi);

    // Apply master music volume from fade in/out
    // FIXME: check whether this should be done after distortion
    if (_audioState._sfxChannels[channel].is_music) {
        volume *= _audioState._musicChannel.volume;
    }

    sample = (int16_t)(32767.99f * volume * waveform);

    // TODO: Apply hardware effects
    if (_memory->hwState.distort & (1 << channel)) {
        sample = sample / 0x1000 * 0x1249;
    }

    _audioState._sfxChannels[channel].phi = phi + freq / samples_per_second;

    _audioState._sfxChannels[channel].offset = next_offset;

    if (next_offset >= 32.f){
        _audioState._sfxChannels[channel].sfxId = -1;
    }
    else if (next_note_idx != note_idx){
        _audioState._sfxChannels[channel].prev_key = sfx.notes[note_idx].getKey();
        _audioState._sfxChannels[channel].prev_vol = sfx.notes[note_idx].getVolume() / 7.f;
    }

    return sample;
}

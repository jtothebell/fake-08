#include "Audio.h"
#include "synth.h"
#include "filter.h"
#include "hostVmShared.h"
#include "mathhelpers.h"

#include <cstdint>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <float.h>
#include <cassert>

//playback implementation based on zepto8's
//https://github.com/samhocevar/zepto8/blob/master/src/pico8/sfx.cpp

Audio::Audio(PicoRam* memory){
    _memory = memory;
    _paused = false;
    
    resetAudioState();
}

void Audio::setPaused(bool paused) {
    _paused = paused;
}

void Audio::resetAudioState() {
    _audioState._musicChannel.count = -1;
    _audioState._musicChannel.pattern = -1;
    _audioState._musicChannel.mask = 0;
    _audioState._musicChannel.volume_music = 0.5f;
    _audioState._musicChannel.volume_sfx = 0.5f;
    _audioState._musicChannel.fade_volume = 0.f;
    _audioState._musicChannel.fade_volume_step = 0.f;
    _audioState._musicChannel.offset = -1;
    _audioState._musicChannel.length = 0;

    for(int i = 0; i < 4; i++) {
        _audioState._sfxChannels[i].main_sfx.sfx = -1;
        _audioState._sfxChannels[i].main_sfx.offset = 0;
        _audioState._sfxChannels[i].main_sfx.time = 0;
        _audioState._sfxChannels[i].main_sfx.prev_key = 24;
        _audioState._sfxChannels[i].main_sfx.prev_vol = 0;
        
        _audioState._sfxChannels[i].custom_sfx.sfx = -1;
        _audioState._sfxChannels[i].sfx_music = -1;
        _audioState._sfxChannels[i].length = 0;
        _audioState._sfxChannels[i].can_loop = true;
        _audioState._sfxChannels[i].is_music = false;
        _audioState._sfxChannels[i].fade = 0.0f;
        _audioState._sfxChannels[i].last_main_instrument = 0;
        _audioState._sfxChannels[i].last_main_key = 0;
        _audioState._sfxChannels[i].reverb_index = 0;
        
        // Reset reverb buffers
        std::memset(_audioState._sfxChannels[i].reverb_2, 0, sizeof(_audioState._sfxChannels[i].reverb_2));
        std::memset(_audioState._sfxChannels[i].reverb_4, 0, sizeof(_audioState._sfxChannels[i].reverb_4));
        
        // Reset damp filters
        _audioState._sfxChannels[i].damp1 = z8::filter(z8::filter::type::highshelf, 2400.0f, 1.0f, -6.0f);
        _audioState._sfxChannels[i].damp2 = z8::filter(z8::filter::type::highshelf, 1000.0f, 1.0f, -12.0f);
    }
}

audioState_t* Audio::getAudioState() {
    return &_audioState;
}

int Audio::api_sfx(int sfx, int channel, int offset, int length){
    // SFX index: valid values are 0..63 for actual samples,
    // -1 to stop sound on a channel, -2 to stop looping on a channel
    // Audio channel: valid values are 0..3, -1 (autoselect), or -2 (stop sfx on any channel)

    if (sfx < -2 || sfx > 63 || channel < -2 || channel > 4 || offset > 31)
        return 0;

    // CHANNEL -2: to stop the given sound from playing on any channel
    if (channel == -2) {
        for (int i = 0; i < 4; ++i) {
            if (_audioState._sfxChannels[i].main_sfx.sfx == sfx) {
                _audioState._sfxChannels[i].main_sfx.sfx = -1;
            }
        }
        return 0;
    }

    if (sfx == -1)
    {
        // Stop playing if sfx is a non musical channel
        if (channel != -1)
        {
            if (!_audioState._sfxChannels[channel].is_music)
                _audioState._sfxChannels[channel].main_sfx.sfx = -1;
        }
        else
        {
            // stop playing all non musical channels
            for (int i = 0; i < 4; ++i)
            {
                if (!_audioState._sfxChannels[i].is_music)
                    _audioState._sfxChannels[i].main_sfx.sfx = -1;
            }
        }
        return 0;
    }

    if (sfx == -2)
    {
        // Stop looping if sfx is a non musical channel
        if (channel != -1)
        {
            if (!_audioState._sfxChannels[channel].is_music)
                _audioState._sfxChannels[channel].can_loop = false;
        }
        else
        {
            // stop looping all non musical channels
            for (int i = 0; i < 4; ++i)
            {
                if (!_audioState._sfxChannels[i].is_music)
                    _audioState._sfxChannels[i].can_loop = false;
            }
        }
        return 0;
    }

    // Find the first available channel: either a channel that plays
    // nothing, or a channel that is already playing this sample (in
    // this case PICO-8 decides to forcibly reuse that channel, which
    // is reasonable)
    if (channel == -1)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (((1 << i) & _audioState._musicChannel.mask) != 0)
                continue;

            if (_audioState._sfxChannels[i].main_sfx.sfx == -1 ||
                _audioState._sfxChannels[i].main_sfx.sfx == sfx)
            {
                channel = i;
                break;
            }
        }
    }

    // if no free channel is found, stop music's first interruptable channel
    if (channel == -1)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (((1 << i) & _audioState._musicChannel.mask) != 0)
                continue;

            if (_audioState._sfxChannels[i].is_music)
            {
                channel = i;
                break;
            }
        }
    }

    // If still no channel found, the PICO-8 strategy seems to be to
    // stop the channel with fastest speed (if there are several, take the latest one)
    if (channel == -1)
    {
        uint8_t fastest_speed = 255;
        for (int i = 0; i < 4; ++i)
        {
            if (((1 << i) & _audioState._musicChannel.mask) != 0)
                continue;

            int const index = _audioState._sfxChannels[i].main_sfx.sfx;
            if (index < 0 || index >= 64)
                continue;
                
            struct sfx const& sfx_data = _memory->sfx[index];
            if (sfx_data.speed <= fastest_speed)
            {
                channel = i;
                fastest_speed = sfx_data.speed;
            }
        }
    }

    // still no channel found, the sfx is ignored
    if (channel == -1)
        return 0;

    // Stop any channel playing the same sfx
    for (int i = 0; i < 4; ++i)
        if (_audioState._sfxChannels[i].main_sfx.sfx == sfx)
            _audioState._sfxChannels[i].main_sfx.sfx = -1;

    // if there is already a music playing sfx, store it to be picked back up later before it's replaced
    if (_audioState._sfxChannels[channel].main_sfx.sfx != -1 && _audioState._sfxChannels[channel].is_music)
    {
        _audioState._sfxChannels[channel].sfx_music = _audioState._sfxChannels[channel].main_sfx.sfx;
    }

    // Play this sound!
    launch_sfx(sfx, channel, (float)std::max(0, offset), (float)std::max(0, length), false);

    return channel;
}

void Audio::api_music(int pattern, int16_t fade_len, int16_t mask){
    // pattern: 0..63, -1 to stop music.
    // fade_len: fade length in milliseconds (default 0)
    // mask: reserved channels

    if (pattern < -1 || pattern > 63)
        return;

    if (pattern == -1)
    {
        // Music will stop when fade out is finished
        _audioState._musicChannel.fade_volume_step = fade_len <= 0 ? -FLT_MAX
                                  : -_audioState._musicChannel.fade_volume * (1000.f / fade_len);
        return;
    }

    // Initialise music state for the whole song
    _audioState._musicChannel.count = 0;
    _audioState._musicChannel.mask = mask & 0xf;

    _audioState._musicChannel.fade_volume = 1.f;
    _audioState._musicChannel.fade_volume_step = 0.f;
    if (fade_len > 0)
    {
        _audioState._musicChannel.fade_volume = 0.f;
        _audioState._musicChannel.fade_volume_step = 1000.f / fade_len;
    }

    set_music_pattern(pattern);
}

void Audio::set_music_pattern(int pattern) {
    using std::max, std::min;

    // stop all previously playing music sounds
    for (int n = 0; n < 4; ++n)
        if (_audioState._sfxChannels[n].is_music)
        {
            _audioState._sfxChannels[n].main_sfx.sfx = -1;
            _audioState._sfxChannels[n].sfx_music = -1;
        }

    if (pattern < 0 || pattern > 63)
    {
        _audioState._musicChannel.pattern = -1;
        _audioState._musicChannel.count = -1;
        _audioState._musicChannel.offset = -1;
        _audioState._musicChannel.mask = 0;
        _audioState._musicChannel.length = 0.0f;
        return;
    }

    // Get song channels
    uint8_t channels[] = {
        _memory->songs[pattern].getSfx0(),
        _memory->songs[pattern].getSfx1(),
        _memory->songs[pattern].getSfx2(),
        _memory->songs[pattern].getSfx3(),
    };

    // Find music duration
    // if there is at least one non-looping channel:
    // length of the first non-looping channel
    // if not (all channels are looping):
    // length of slowest channel (so it stops when all channels have reached at least 32 steps)

    int16_t duration_looping = -1;
    int16_t duration_no_loop = -1;
    for (int i = 0; i < 4; ++i)
    {
        int n = channels[i];
        if (n & 0x40)
            continue;

        auto &sfx_data = _memory->sfx[n & 0x3f];
        bool has_loop = sfx_data.loopRangeEnd > 0 && sfx_data.loopRangeEnd > sfx_data.loopRangeStart;
        if (has_loop)
        {
            int16_t sfx_duration = 32 * sfx_data.speed;
            duration_looping = max(duration_looping, sfx_duration);
        }
        else
        {
            // take duration of first non_looping channel
            int16_t end_time = 32;
            if (sfx_data.loopRangeEnd == 0 && sfx_data.loopRangeStart > 0)
            {
                end_time = min<int16_t>(end_time, sfx_data.loopRangeStart);
            }
            duration_no_loop = end_time * sfx_data.speed;
            break;
        }
    }

    int16_t duration = duration_no_loop > 0 ? duration_no_loop : duration_looping;
    if (duration <= 0)
    {
        // Default duration if no valid sfx found
        duration = 32;
    }

    // Initialise music state for the current pattern
    _audioState._musicChannel.pattern = pattern;
    _audioState._musicChannel.offset = 0;
    _audioState._musicChannel.length = (float)duration;

    // Play music sfx on active channels
    for (int i = 0; i < 4; ++i)
    {
        int n = channels[i];
        if (n & 0x40)
            continue;

        if (_audioState._sfxChannels[i].main_sfx.sfx == -1)
        {
            launch_sfx(n, i, 0, 0, true);
        }
        else
        {
            // if there is already a sfx playing, we store the music one to be played later, when the current sfx stop
            _audioState._sfxChannels[i].sfx_music = n;
        }
    }
}

void Audio::launch_sfx(int16_t sfx, int16_t chan, float offset, float length, bool is_music)
{
    _audioState._sfxChannels[chan].main_sfx.sfx = sfx;
    _audioState._sfxChannels[chan].main_sfx.offset = std::max(0.f, offset);
    _audioState._sfxChannels[chan].main_sfx.time = 0.f;
    _audioState._sfxChannels[chan].length = std::max(0.f, length);
    _audioState._sfxChannels[chan].can_loop = true;
    _audioState._sfxChannels[chan].is_music = is_music;
    _audioState._sfxChannels[chan].last_main_instrument = 0xff;
    _audioState._sfxChannels[chan].last_main_key = 0xff;
    // Playing an instrument starting with the note C-2 and the
    // slide effect causes no noticeable pitch variation in PICO-8,
    // so I assume this is the default value for "previous key".
    _audioState._sfxChannels[chan].main_sfx.prev_key = 24;
    // There is no default value for "previous volume".
    _audioState._sfxChannels[chan].main_sfx.prev_vol = 0.f;
}

static float key_to_freq(float key)
{
    using std::exp2;
    return 440.f * exp2((key - 33.f) / 12.f);
}

int16_t Audio::getCurrentSfxId(int channel){
    return _audioState._sfxChannels[channel].main_sfx.sfx;
}

int Audio::getCurrentNoteNumber(int channel){
    return _audioState._sfxChannels[channel].main_sfx.sfx < 0 
        ? -1
        : (int)_audioState._sfxChannels[channel].main_sfx.offset;
}

int16_t Audio::getCurrentMusic(){
    return _audioState._musicChannel.pattern;
}

int16_t Audio::getMusicPatternCount(){
    return _audioState._musicChannel.count;
}

int16_t Audio::getMusicTickCount(){
    return (int16_t)_audioState._musicChannel.offset;
}

void Audio::update_sfx_state(sfx_state& cur_sfx, z8::synth_param& new_synth, 
                              float freq_factor, float length, bool is_music, 
                              bool can_loop, bool half_rate, double inv_frames_per_second)
{
    using std::fabs, std::fmod, std::floor, std::max;

    if (cur_sfx.sfx == -1) return;

    int const index = cur_sfx.sfx;
    assert(index >= 0 && index < 64);
    struct sfx const& sfx_data = _memory->sfx[index];

    // Speed must be 1—255 otherwise the SFX is invalid
    int const speed = max(1, (int)sfx_data.speed);

    double const offset = cur_sfx.offset;
    double const time = cur_sfx.time;

    // PICO-8 exports instruments as 22050 Hz WAV files with 183 samples
    // per speed unit per note, so this is how much we should advance
    double const offset_per_second = 22050.0 / (183.0 * speed);
    double const offset_per_frame = offset_per_second * inv_frames_per_second;
    double next_offset = offset + offset_per_frame;
    double next_time = time + offset_per_frame;

    // Handle SFX loops. From the documentation: "Looping is turned
    // off when the start index >= end index".
    float const loop_range = float(sfx_data.loopRangeEnd - sfx_data.loopRangeStart);
    if (loop_range > 0.f && next_offset >= sfx_data.loopRangeEnd && can_loop)
    {
        next_offset = fmod(next_offset - sfx_data.loopRangeStart, loop_range)
            + sfx_data.loopRangeStart;
    }

    bool has_end = false;
    float end_time = 32.f;
    if (length > 0.0f)
    {
        has_end = true;
        end_time = length;
    }
    // in pico 8, strangely, len is not applied to musical sfx except for pattern len calculation
    // it's probably a bug
    if (!is_music && sfx_data.loopRangeEnd == 0 && sfx_data.loopRangeStart > 0)
    {
        has_end = true;
        end_time = std::min<float>(end_time, sfx_data.loopRangeStart);
    }
    // if there is no loop, we end after the length
    if (loop_range <= 0.f)
    {
        has_end = true;
        // if not a music sfx, check where is the last note to early stop
        if (!is_music)
        {
            int last_note = 0;
            for (int n = 0; n < 32; ++n)
            {
                if (sfx_data.notes[n].getVolume() > 0)
                {
                    last_note = std::min(32, n + 1);
                }
            }
            end_time = std::min(end_time, float(last_note));
        }
    }

    if (offset < 32)
    {
        int const note_id = (int)floor(offset);
        int const next_note_id = (int)floor(next_offset);

        uint8_t key = sfx_data.notes[note_id].getKey();
        float volume = sfx_data.notes[note_id].getVolume() / 7.f;
        float freq = key_to_freq(key) * freq_factor;

        if (volume > 0.f)
        {
            int const fx = sfx_data.notes[note_id].getEffect();

            // Apply effect, if any
            switch (fx)
            {
            case FX_NO_EFFECT:
                break;
            case FX_SLIDE:
            {
                float t = (float)fmod(offset, 1.0);
                // From the documentation: "Slide to the next note and volume",
                // but it's actually _from_ the _prev_ note and volume.
                freq = lerp(key_to_freq((float)cur_sfx.prev_key), freq, t);
                if (cur_sfx.prev_vol > 0.f)
                    volume = lerp(cur_sfx.prev_vol, volume, t);
                break;
            }
            case FX_VIBRATO:
            {
                // 7.5f and 0.25f were found empirically by matching
                // frequency graphs of PICO-8 instruments.
                float t = (float)(fabs(fmod(7.5 * offset / offset_per_second, 1.0)) - 0.5) - 0.25f;
                // Vibrato half a semi-tone, so multiply by pow(2,1/12)
                freq = lerp(freq, freq * 1.059463094359f, t);
                break;
            }
            case FX_DROP:
                freq *= 1.f - (float)fmod(offset, 1.0);
                break;
            case FX_FADE_IN:
                volume *= (float)fmod(offset, 1.0);
                break;
            case FX_FADE_OUT:
                volume *= 1.f - (float)fmod(offset, 1.0);
                break;
            case FX_ARP_FAST:
            case FX_ARP_SLOW:
            {
                // From the documentation:
                // "6 arpeggio fast  //  Iterate over groups of 4 notes at speed of 4
                //  7 arpeggio slow  //  Iterate over groups of 4 notes at speed of 8"
                // "If the SFX speed is <= 8, arpeggio speeds are halved to 2, 4"
                int const m = (speed <= 8 ? 32 : 16) / (fx == FX_ARP_FAST ? 4 : 8);
                int const n = (int)(m * 7.5 * offset / offset_per_second);
                int const arp_note = (note_id & ~3) | (n & 3);
                freq = key_to_freq(sfx_data.notes[arp_note].getKey());
                break;
            }
            }

            if (half_rate) freq *= 0.5f;

            new_synth.key = key;
            new_synth.freq = freq;
            new_synth.instrument = sfx_data.notes[note_id].getWaveform();
            new_synth.custom = sfx_data.notes[note_id].getCustom();
            new_synth.filters = sfx_data.filters;
            new_synth.volume = volume;
            new_synth.is_music = is_music;

            new_synth.phi = new_synth.phi + (float)(freq * inv_frames_per_second);
        }

        if (next_note_id != note_id)
        {
            cur_sfx.prev_key = sfx_data.notes[note_id].getKey();
            cur_sfx.prev_vol = sfx_data.notes[note_id].getVolume() / 7.f;
        }
    }

    cur_sfx.offset = next_offset;
    cur_sfx.time = next_time;

    if (has_end && next_time >= end_time)
    {
        cur_sfx.sfx = -1;
    }
}

float Audio::get_synth_sample(z8::synth_param& params)
{
    // Play note
    float waveform = z8::synth::waveform(params);

    uint8_t detune = (params.filters / 8) % 3;
    if (detune != 0 && params.instrument != z8::synth::INST_NOISE)
    {
        // detune is a second wave slightly offset
        float factor = 1.0f;
        if (params.instrument == z8::synth::INST_TRIANGLE) 
            factor = (detune == 1) ? 3.0f / 4.0f : 3.0f / 2.0f; // triangle detune adds a fourth or a fifth
        else if (params.instrument == z8::synth::INST_ORGAN) 
            factor = (detune == 1) ? 200.0f / 199.0f : 800.0f / 199.0f; // slight offset, detune 2 at 2 octave above
        else if (params.instrument == z8::synth::INST_PHASER) 
            factor = (detune == 1) ? 49.0f / 50.0f : 400.0f / 199.0f;
        else 
            factor = (detune == 1) ? 200.0f / 199.0f : 400.0f / 199.0f; // others are slight offset, detune 2 at 1 octave above

        z8::synth_param second_wave = params;
        second_wave.phi *= factor;
        if (detune == 2 && params.instrument == z8::synth::INST_ORGAN) 
            second_wave.instrument = z8::synth::INST_TRIANGLE; // organ second wave seems to be simpler
        waveform += z8::synth::waveform(second_wave) * 0.5f;
    }

    float volume = params.volume;

    // Apply master music volume from fade in/out
    if (params.is_music)
    {
        volume *= _audioState._musicChannel.fade_volume * _audioState._musicChannel.volume_music;
    }
    else
    {
        volume *= _audioState._musicChannel.volume_sfx;
    }

    return std::clamp(waveform * volume, -1.0f, 1.0f);
}

void Audio::FillAudioBuffer(void *audioBuffer, size_t offset, size_t size){
    if (audioBuffer == nullptr) {
        return;
    }

    uint32_t *buffer = (uint32_t *)audioBuffer;

    // Output silence when paused
    if (_paused) {
        for (size_t i = 0; i < size; ++i){
            buffer[i] = 0;
        }
        return;
    }

    for (size_t i = 0; i < size; ++i){
        int32_t sample = 0;
        float channel_mix = 0.0f;

        bool is_pause = _memory->drawState.soundPauseState == 1;

        for (int chan = 0; chan < 4; ++chan) {
            double inv_frames_per_second = ((_memory->hwState.half_rate & (1 << chan)) ? 0.5 : 1.0) / 22050.0;

            sfxChannel& channel_state = _audioState._sfxChannels[chan];

            // Advance music using the first channel
            if (chan == 0 && _audioState._musicChannel.pattern != -1 && !is_pause)
            {
                double const offset_per_second = 22050.0 / 183.0;
                double const offset_per_frame = offset_per_second * inv_frames_per_second;
                _audioState._musicChannel.offset += offset_per_frame;
                _audioState._musicChannel.fade_volume += (float)(_audioState._musicChannel.fade_volume_step * inv_frames_per_second);
                _audioState._musicChannel.fade_volume = clamp(_audioState._musicChannel.fade_volume, 0.f, 1.f);

                if (_audioState._musicChannel.fade_volume_step < 0 && _audioState._musicChannel.fade_volume <= 0)
                {
                    set_music_pattern(-1);
                }
                else if (_audioState._musicChannel.offset >= _audioState._musicChannel.length)
                {
                    int16_t next_pattern = _audioState._musicChannel.pattern + 1;
                    int16_t next_count = _audioState._musicChannel.count + 1;
                    if (_memory->songs[_audioState._musicChannel.pattern].getStop())
                    {
                        next_pattern = -1;
                        next_count = -1;
                    }
                    else if (_memory->songs[_audioState._musicChannel.pattern].getLoop())
                        while (--next_pattern > 0 && !_memory->songs[next_pattern].getStart())
                            ;

                    _audioState._musicChannel.count = next_count;
                    set_music_pattern(next_pattern);
                }
            }

            // if no sfx is playing and there is a music sfx stored
            if (channel_state.main_sfx.sfx == -1 && channel_state.sfx_music != -1 && !is_pause)
            {
                int const index = channel_state.sfx_music;
                assert(index >= 0 && index < 64);
                struct sfx const& sfx_data = _memory->sfx[index];

                // compute offset to start the sfx to
                bool want_play = true;
                int const speed = std::max(1, (int)sfx_data.speed);
                double new_offset = _audioState._musicChannel.offset / speed;

                float const loop_range = (float)(sfx_data.loopRangeEnd - sfx_data.loopRangeStart);
                if (loop_range > 0.f && channel_state.can_loop)
                {
                    if (new_offset > sfx_data.loopRangeStart)
                        new_offset = std::fmod(new_offset - sfx_data.loopRangeStart, loop_range) + sfx_data.loopRangeStart;
                }
                else
                {
                    if (new_offset > 32.0)
                        want_play = false;
                }

                if (want_play)
                {
                    launch_sfx(index, chan, (float)new_offset, 0, true);
                }
                channel_state.sfx_music = -1;
            }

            z8::synth_param& last_synth = channel_state.last_synth;
            z8::synth_param new_synth;
            new_synth.phi = last_synth.phi;
            new_synth.last_advance = last_synth.last_advance;
            new_synth.last_sample = last_synth.last_sample;
            float value = 0.0f;

            if (!is_pause)
            {
                double main_sfx_base_offset = channel_state.main_sfx.offset;
                bool half_rate = _memory->hwState.half_rate & (1 << (chan + 4));
                // update main sfx
                update_sfx_state(channel_state.main_sfx, new_synth, 1.0f, channel_state.length, 
                                channel_state.is_music, channel_state.can_loop, half_rate, inv_frames_per_second);

                bool restart_custom = new_synth.instrument != channel_state.last_main_instrument || 
                                      new_synth.key != channel_state.last_main_key;
                channel_state.last_main_instrument = new_synth.instrument;
                channel_state.last_main_key = new_synth.key;

                if (new_synth.volume > 0.0f)
                {
                    if (new_synth.custom)
                    {
                        // also need to restart if main_sfx loops (new offset is before base offset)
                        if (channel_state.main_sfx.offset < main_sfx_base_offset) restart_custom = true;
                        // also need to restart if custom_sfx.sfx == -1 (it has ended) and main_sfx.offset is changing integer
                        if (channel_state.custom_sfx.sfx == -1 && 
                            std::floor(main_sfx_base_offset) != std::floor(channel_state.main_sfx.offset)) 
                            restart_custom = true;

                        if (restart_custom)
                        {
                            channel_state.custom_sfx.sfx = new_synth.instrument;
                            channel_state.custom_sfx.offset = 0.0;
                            channel_state.custom_sfx.time = 0.0;
                        }
                        new_synth.phi = last_synth.phi;
                        float const freq_base = key_to_freq(24); // C2
                        float freq_factor = new_synth.freq / freq_base;
                        float main_sfx_volume = new_synth.volume;
                        update_sfx_state(channel_state.custom_sfx, new_synth, freq_factor, 0.0f, false, true, half_rate, inv_frames_per_second);
                        new_synth.volume *= main_sfx_volume;
                    }
                    value = get_synth_sample(new_synth);
                }
            }

            // detect harsh changes of states, and do a small fade
            float freq_threshold = std::min(new_synth.freq, last_synth.freq) * 0.01f;
            if (std::abs(new_synth.volume - last_synth.volume) > 0.1f
                || std::abs(new_synth.freq - last_synth.freq) > freq_threshold
                || new_synth.instrument != last_synth.instrument)
            {
                if (channel_state.fade <= 0.0f) // avoid continuous fades, it messes with noise algo
                {
                    channel_state.fade_synth = last_synth;
                }
                channel_state.fade = 1.0f;
                // reset phi between notes so we don't get very big values that would lose precision
                new_synth.phi = std::fmod(new_synth.phi, 1.0f);
            }
            last_synth = new_synth;

            uint8_t reverb = (last_synth.filters / 24) % 3;
            uint8_t dampen = (last_synth.filters / 72) % 3;
            float chan_reverb1_value = reverb == 1 ? 1.0f : 0.0f;
            float chan_reverb2_value = reverb == 2 ? 1.0f : 0.0f;
            float chan_damp1_value = dampen == 1 ? 1.0f : 0.0f;
            float chan_damp2_value = dampen == 2 ? 1.0f : 0.0f;

            if (channel_state.fade > 0.0f)
            {
                channel_state.fade_synth.phi = channel_state.fade_synth.phi + 
                    (float)(channel_state.fade_synth.freq * inv_frames_per_second);
                float value_fade = get_synth_sample(channel_state.fade_synth);
                
                value = lerp(value, value_fade, channel_state.fade);

                // Also mix fade filter values
                uint8_t fade_reverb = (channel_state.fade_synth.filters / 24) % 3;
                uint8_t fade_dampen = (channel_state.fade_synth.filters / 72) % 3;
                chan_reverb1_value = lerp(chan_reverb1_value, fade_reverb == 1 ? 1.0f : 0.0f, channel_state.fade);
                chan_reverb2_value = lerp(chan_reverb2_value, fade_reverb == 2 ? 1.0f : 0.0f, channel_state.fade);
                chan_damp1_value = lerp(chan_damp1_value, fade_dampen == 1 ? 1.0f : 0.0f, channel_state.fade);
                chan_damp2_value = lerp(chan_damp2_value, fade_dampen == 2 ? 1.0f : 0.0f, channel_state.fade);

                channel_state.fade -= (float)(130.0 * inv_frames_per_second);
            }

            // hw can force fx passes
            if (_memory->hwState.reverb & (1 << (chan + 4))) chan_reverb1_value = 1.0f;
            if (_memory->hwState.reverb & (1 << chan)) chan_reverb2_value = 1.0f;
            if (_memory->hwState.lowpass & (1 << (chan + 4))) chan_damp1_value = 1.0f;
            if (_memory->hwState.lowpass & (1 << chan)) chan_damp2_value = 1.0f;
            
            if (chan_reverb1_value > 0.0f) 
                value += chan_reverb1_value * channel_state.reverb_2[channel_state.reverb_index % 366] * 0.5f;
            if (chan_reverb2_value > 0.0f) 
                value += chan_reverb2_value * channel_state.reverb_4[channel_state.reverb_index % 732] * 0.5f;

            channel_state.reverb_2[channel_state.reverb_index % 366] = value;
            channel_state.reverb_4[channel_state.reverb_index % 732] = value;
            ++channel_state.reverb_index;

            float value_damp1 = channel_state.damp1.run(value);
            if (chan_damp1_value > 0.0f) value = lerp(value, value_damp1, chan_damp1_value);

            float value_damp2 = channel_state.damp2.run(value);
            if (chan_damp2_value > 0.0f) value = lerp(value, value_damp2, chan_damp2_value);

            int16_t chan_sample = (int16_t)(32767.99f * std::clamp(value, -0.99f, 0.99f));

            // Apply hardware distort
            if (_memory->hwState.distort & (1 << chan))
            {
                chan_sample = chan_sample / 0x1000 * 0x1249;
            }
            else if (_memory->hwState.distort & (1 << (chan + 4)))
            {
                chan_sample = (chan_sample - (chan_sample < 0 ? 0x1000 : 0)) / 0x1000 * 0x1249;
            }
            channel_mix += chan_sample;
        }

        sample = (int32_t)std::clamp(channel_mix, -32767.9f, 32767.9f);

        //buffer is stereo, so just send the mono sample to both channels
        buffer[i] = ((uint32_t)(uint16_t)sample << 16) | ((uint16_t)sample);
    }
}


void Audio::FillMonoAudioBuffer(void *audioBuffer, size_t offset, size_t size){
    if (audioBuffer == nullptr) {
        return;
    }

    int16_t *buffer = (int16_t *)audioBuffer;

    // Output silence when paused
    if (_paused) {
        for (size_t i = 0; i < size; ++i){
            buffer[i] = 0;
        }
        return;
    }

    for (size_t i = 0; i < size; ++i){
        float channel_mix = 0.0f;

        bool is_pause = _memory->drawState.soundPauseState == 1;

        for (int chan = 0; chan < 4; ++chan) {
            double inv_frames_per_second = ((_memory->hwState.half_rate & (1 << chan)) ? 0.5 : 1.0) / 22050.0;

            sfxChannel& channel_state = _audioState._sfxChannels[chan];

            // Advance music using the first channel
            if (chan == 0 && _audioState._musicChannel.pattern != -1 && !is_pause)
            {
                double const offset_per_second = 22050.0 / 183.0;
                double const offset_per_frame = offset_per_second * inv_frames_per_second;
                _audioState._musicChannel.offset += offset_per_frame;
                _audioState._musicChannel.fade_volume += (float)(_audioState._musicChannel.fade_volume_step * inv_frames_per_second);
                _audioState._musicChannel.fade_volume = clamp(_audioState._musicChannel.fade_volume, 0.f, 1.f);

                if (_audioState._musicChannel.fade_volume_step < 0 && _audioState._musicChannel.fade_volume <= 0)
                {
                    set_music_pattern(-1);
                }
                else if (_audioState._musicChannel.offset >= _audioState._musicChannel.length)
                {
                    int16_t next_pattern = _audioState._musicChannel.pattern + 1;
                    int16_t next_count = _audioState._musicChannel.count + 1;
                    if (_memory->songs[_audioState._musicChannel.pattern].getStop())
                    {
                        next_pattern = -1;
                        next_count = -1;
                    }
                    else if (_memory->songs[_audioState._musicChannel.pattern].getLoop())
                        while (--next_pattern > 0 && !_memory->songs[next_pattern].getStart())
                            ;

                    _audioState._musicChannel.count = next_count;
                    set_music_pattern(next_pattern);
                }
            }

            // if no sfx is playing and there is a music sfx stored
            if (channel_state.main_sfx.sfx == -1 && channel_state.sfx_music != -1 && !is_pause)
            {
                int const index = channel_state.sfx_music;
                assert(index >= 0 && index < 64);
                struct sfx const& sfx_data = _memory->sfx[index];

                bool want_play = true;
                int const speed = std::max(1, (int)sfx_data.speed);
                double new_offset = _audioState._musicChannel.offset / speed;

                float const loop_range = (float)(sfx_data.loopRangeEnd - sfx_data.loopRangeStart);
                if (loop_range > 0.f && channel_state.can_loop)
                {
                    if (new_offset > sfx_data.loopRangeStart)
                        new_offset = std::fmod(new_offset - sfx_data.loopRangeStart, loop_range) + sfx_data.loopRangeStart;
                }
                else
                {
                    if (new_offset > 32.0)
                        want_play = false;
                }

                if (want_play)
                {
                    launch_sfx(index, chan, (float)new_offset, 0, true);
                }
                channel_state.sfx_music = -1;
            }

            z8::synth_param& last_synth = channel_state.last_synth;
            z8::synth_param new_synth;
            new_synth.phi = last_synth.phi;
            new_synth.last_advance = last_synth.last_advance;
            new_synth.last_sample = last_synth.last_sample;
            float value = 0.0f;

            if (!is_pause)
            {
                double main_sfx_base_offset = channel_state.main_sfx.offset;
                bool half_rate = _memory->hwState.half_rate & (1 << (chan + 4));
                update_sfx_state(channel_state.main_sfx, new_synth, 1.0f, channel_state.length, 
                                channel_state.is_music, channel_state.can_loop, half_rate, inv_frames_per_second);

                bool restart_custom = new_synth.instrument != channel_state.last_main_instrument || 
                                      new_synth.key != channel_state.last_main_key;
                channel_state.last_main_instrument = new_synth.instrument;
                channel_state.last_main_key = new_synth.key;

                if (new_synth.volume > 0.0f)
                {
                    if (new_synth.custom)
                    {
                        if (channel_state.main_sfx.offset < main_sfx_base_offset) restart_custom = true;
                        if (channel_state.custom_sfx.sfx == -1 && 
                            std::floor(main_sfx_base_offset) != std::floor(channel_state.main_sfx.offset)) 
                            restart_custom = true;

                        if (restart_custom)
                        {
                            channel_state.custom_sfx.sfx = new_synth.instrument;
                            channel_state.custom_sfx.offset = 0.0;
                            channel_state.custom_sfx.time = 0.0;
                        }
                        new_synth.phi = last_synth.phi;
                        float const freq_base = key_to_freq(24);
                        float freq_factor = new_synth.freq / freq_base;
                        float main_sfx_volume = new_synth.volume;
                        update_sfx_state(channel_state.custom_sfx, new_synth, freq_factor, 0.0f, false, true, half_rate, inv_frames_per_second);
                        new_synth.volume *= main_sfx_volume;
                    }
                    value = get_synth_sample(new_synth);
                }
            }

            float freq_threshold = std::min(new_synth.freq, last_synth.freq) * 0.01f;
            if (std::abs(new_synth.volume - last_synth.volume) > 0.1f
                || std::abs(new_synth.freq - last_synth.freq) > freq_threshold
                || new_synth.instrument != last_synth.instrument)
            {
                if (channel_state.fade <= 0.0f)
                {
                    channel_state.fade_synth = last_synth;
                }
                channel_state.fade = 1.0f;
                new_synth.phi = std::fmod(new_synth.phi, 1.0f);
            }
            last_synth = new_synth;

            uint8_t reverb = (last_synth.filters / 24) % 3;
            uint8_t dampen = (last_synth.filters / 72) % 3;
            float chan_reverb1_value = reverb == 1 ? 1.0f : 0.0f;
            float chan_reverb2_value = reverb == 2 ? 1.0f : 0.0f;
            float chan_damp1_value = dampen == 1 ? 1.0f : 0.0f;
            float chan_damp2_value = dampen == 2 ? 1.0f : 0.0f;

            if (channel_state.fade > 0.0f)
            {
                channel_state.fade_synth.phi = channel_state.fade_synth.phi + 
                    (float)(channel_state.fade_synth.freq * inv_frames_per_second);
                float value_fade = get_synth_sample(channel_state.fade_synth);
                
                value = lerp(value, value_fade, channel_state.fade);

                uint8_t fade_reverb = (channel_state.fade_synth.filters / 24) % 3;
                uint8_t fade_dampen = (channel_state.fade_synth.filters / 72) % 3;
                chan_reverb1_value = lerp(chan_reverb1_value, fade_reverb == 1 ? 1.0f : 0.0f, channel_state.fade);
                chan_reverb2_value = lerp(chan_reverb2_value, fade_reverb == 2 ? 1.0f : 0.0f, channel_state.fade);
                chan_damp1_value = lerp(chan_damp1_value, fade_dampen == 1 ? 1.0f : 0.0f, channel_state.fade);
                chan_damp2_value = lerp(chan_damp2_value, fade_dampen == 2 ? 1.0f : 0.0f, channel_state.fade);

                channel_state.fade -= (float)(130.0 * inv_frames_per_second);
            }

            if (_memory->hwState.reverb & (1 << (chan + 4))) chan_reverb1_value = 1.0f;
            if (_memory->hwState.reverb & (1 << chan)) chan_reverb2_value = 1.0f;
            if (_memory->hwState.lowpass & (1 << (chan + 4))) chan_damp1_value = 1.0f;
            if (_memory->hwState.lowpass & (1 << chan)) chan_damp2_value = 1.0f;
            
            if (chan_reverb1_value > 0.0f) 
                value += chan_reverb1_value * channel_state.reverb_2[channel_state.reverb_index % 366] * 0.5f;
            if (chan_reverb2_value > 0.0f) 
                value += chan_reverb2_value * channel_state.reverb_4[channel_state.reverb_index % 732] * 0.5f;

            channel_state.reverb_2[channel_state.reverb_index % 366] = value;
            channel_state.reverb_4[channel_state.reverb_index % 732] = value;
            ++channel_state.reverb_index;

            float value_damp1 = channel_state.damp1.run(value);
            if (chan_damp1_value > 0.0f) value = lerp(value, value_damp1, chan_damp1_value);

            float value_damp2 = channel_state.damp2.run(value);
            if (chan_damp2_value > 0.0f) value = lerp(value, value_damp2, chan_damp2_value);

            int16_t chan_sample = (int16_t)(32767.99f * std::clamp(value, -0.99f, 0.99f));

            if (_memory->hwState.distort & (1 << chan))
            {
                chan_sample = chan_sample / 0x1000 * 0x1249;
            }
            else if (_memory->hwState.distort & (1 << (chan + 4)))
            {
                chan_sample = (chan_sample - (chan_sample < 0 ? 0x1000 : 0)) / 0x1000 * 0x1249;
            }
            channel_mix += chan_sample;
        }

        buffer[i] = (int16_t)std::clamp(channel_mix, -32767.9f, 32767.9f);
    }
}

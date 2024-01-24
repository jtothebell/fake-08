#include "Audio.h"
#include "synth.h"
#include "hostVmShared.h"
#include "mathhelpers.h"

#include <cstdint>
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
        _audioState._sfxChannels[i].current_note.phi = 0;
        _audioState._sfxChannels[i].can_loop = true;
        _audioState._sfxChannels[i].is_music = false;
        _audioState._sfxChannels[i].prev_note.n.setKey(0);
        _audioState._sfxChannels[i].prev_note.n.setVolume(0);
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

audioState_t* Audio::getAudioState() {
    return &_audioState;
}

void Audio::api_sfx(int sfx, int channel, int offset){

    if (sfx < -2 || sfx > 63 || channel < -2 || channel > 3 || offset > 31) {
        return;
    }

    //CHANNEL -2: to stop the given sound from playing on any channel
    if (channel == -2) {
        for(int i = 0; i < 4; i++) {
            if (_audioState._sfxChannels[i].sfxId == sfx) {
                _audioState._sfxChannels[i].sfxId = -1;
            }
        }
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
        _audioState._sfxChannels[channel].current_note.phi = 0.f;
        _audioState._sfxChannels[channel].can_loop = true;
        _audioState._sfxChannels[channel].is_music = false;
        // Playing an instrument starting with the note C-2 and the
        // slide effect causes no noticeable pitch variation in PICO-8,
        // so I assume this is the default value for “previous key”.
        _audioState._sfxChannels[channel].prev_note.n.setKey(24);
        // There is no default value for “previous volume”.
        _audioState._sfxChannels[channel].prev_note.n.setVolume(0);
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

    uint16_t shortest = 32 * 255; // longest sfx possible
    bool foundNonLooping = false;
    // Find music speed; it’s the speed of the fastest sfx
	// While we are looping through this, find the lowest *valid* sfx length
    _audioState._musicChannel.master = -1;
    _audioState._musicChannel.speed = 0;
	_audioState._musicChannel.length = 32; //as far as i can tell, there is no way to make an sfx longer than 32.
    for (int i = 0; i < 4; ++i)
    {
        uint8_t n = channels[i];

        if (n & 0x40)
            continue;
	// we ignore loooping length if we have non-looping channel
        auto &sfx = _memory->sfx[n & 0x3f];
        bool looping = sfx.loopRangeStart < sfx.loopRangeEnd;
	bool firstNonLooping = !looping && !foundNonLooping;
	if (!looping) {
		foundNonLooping=true;
	}

	uint8_t length = 32;
	if(sfx.loopRangeStart != 0 && sfx.loopRangeEnd == 0)
	{
		length= sfx.loopRangeStart;			
	}
	else if (sfx.loopRangeEnd > sfx.loopRangeStart) {
		// length = sfx.loopRangeEnd;
    // pico 8 pretends like it's length 32!
	}

	uint16_t timeLength = length * std::max(1, (int)sfx.speed);;

        if ((!looping || !foundNonLooping) && (firstNonLooping || _audioState._musicChannel.master == -1 || shortest > timeLength))
        {
	    shortest = timeLength;
            _audioState._musicChannel.master = i;
            _audioState._musicChannel.speed = std::max(1, (int)sfx.speed);
	    _audioState._musicChannel.length = length;
        }
    }
	
	

    // Play music sfx on active channels
    for (int i = 0; i < 4; ++i)
    {
        if (((1 << i) & _audioState._musicChannel.mask) == 0)
            continue;

        uint8_t n = channels[i];
        if (n & 0x40) {
            // Make sure this channel will be silent
            _audioState._sfxChannels[i].sfxId = -1;
            continue;
        }

        _audioState._sfxChannels[i].sfxId = n;
        _audioState._sfxChannels[i].offset = 0.f;
        _audioState._sfxChannels[i].current_note.phi = 0.f;
	// if the master channel loops we'll never finish
        _audioState._sfxChannels[i].can_loop = i != _audioState._musicChannel.master;
        _audioState._sfxChannels[i].is_music = true;
        _audioState._sfxChannels[i].prev_note.n.setKey(24);
        _audioState._sfxChannels[i].prev_note.n.setVolume(0);
    }
}

void Audio::FillAudioBuffer(void *audioBuffer, size_t offset, size_t size){
    if (audioBuffer == nullptr) {
        return;
    }

    uint32_t *buffer = (uint32_t *)audioBuffer;

    for (size_t i = 0; i < size; ++i){
        int32_t sample = 0;

        for (int c = 0; c < 4; ++c) {
            sample += this->getSampleForChannel(c);
        }

	    if (sample > 0x7fff) sample = 0x7fff; else if (sample < -0x8000) sample = -0x8000;

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
        int32_t sample = 0;

        for (int c = 0; c < 4; ++c) {
            sample += this->getSampleForChannel(c);
        }

	    if (sample > 0x7fff) sample = 0x7fff; else if (sample < -0x8000) sample = -0x8000;

        buffer[i] = sample;
    }
}

static float key_to_freq(float key)
{
    using std::exp2;
    return 440.f * exp2((key - 33.f) / 12.f);
}

const float C2_FREQ = key_to_freq(24);

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

float Audio::getSampleForSfx(rawSfxChannel &channel, float freqShift) {
    using std::max;
    int const samples_per_second = 22050;

    if (channel.sfxId < 0 || channel.sfxId > 63) {
        //no (valid) sfx here. return silence
        return 0;
    }
    struct sfx const &sfx = _memory->sfx[channel.sfxId];

    // Speed must be 1—255 otherwise the SFX is invalid
    int const speed = max(1, (int)sfx.speed);

    // PICO-8 exports instruments as 22050 Hz WAV files with 183 samples
    // per speed unit per note, so this is how much we should advance
    float const offset_per_second = 22050.f / (183.f * speed);
    float const offset_per_sample = offset_per_second / samples_per_second;
    float next_offset = channel.offset + offset_per_sample;

    // Handle SFX loops. From the documentation: “Looping is turned
    // off when the start index >= end index”.
    float const loop_range = float(sfx.loopRangeEnd - sfx.loopRangeStart);
    if (loop_range > 0.f && next_offset >= sfx.loopRangeStart && channel.can_loop) {
        next_offset = fmod(next_offset - sfx.loopRangeStart, loop_range)
                    + sfx.loopRangeStart;
    }

    int const note_idx = (int)floor(channel.offset);
    channel.current_note.n=sfx.notes[note_idx];
    int const next_note_idx = (int)floor(next_offset);



    /*
    if (volume == 0.f){
        //volume all the way off. return silence, but make sure to set stuff
        channel.offset = next_offset;

        if (next_offset >= 32.f){
            channel.sfxId = -1;
            if (channel.getChildChannel()) {
              channel.getChildChannel()->sfxId = -1;
            }
        }
        else if (next_note_idx != note_idx){
            channel.prev_note = sfx.notes[note_idx];
        }

        return 0;
    }
    */

    // tiniest fade in/out to fix popping
    // the real version uses a crossfade it looks like
    // 25 samples was estimated from looking at pcm out from pico-8
    float const fade_duration = offset_per_sample * 25;
    float offset_part = fmod(channel.offset, 1.f);
    float crossfade = 0;
    if (offset_part < fade_duration) {
      crossfade = (fade_duration-offset_part)/fade_duration;
    }
    

    bool custom = (bool) sfx.notes[note_idx].getCustom() && channel.getChildChannel() != NULL; 
    // it seems we're not allowed to play custom instruments
    // recursively inside a custom instrument.
    float waveform = this->getSampleForNote(channel.current_note, channel, channel.getChildChannel(), channel.prev_note.n, freqShift, false);
    if (crossfade > 0) {
      waveform *= (1.0f-crossfade);
      note dummyNote;
      waveform+= crossfade * this->getSampleForNote(channel.prev_note, channel, channel.getPrevChildChannel(), dummyNote, freqShift, true);
    }
    uint8_t len = sfx.loopRangeEnd == 0 ? 32 : sfx.loopRangeEnd;
    bool lastNote = note_idx == len - 1;
    if (lastNote && 1.0f - offset_part < fade_duration) {
      waveform *= (fade_duration - 1.0f + offset_part)/fade_duration;
    }

    // Apply master music volume from fade in/out
    // FIXME: check whether this should be done after distortion
    if (channel.is_music) {
        waveform *= _audioState._musicChannel.volume;
    }

    channel.offset = next_offset;

    if (next_offset >= 32.f){
        channel.sfxId = -1;
        if (custom) {
          channel.getChildChannel()->sfxId = -1;
        }
    }
    else if (next_note_idx != note_idx){
        channel.prev_note = channel.current_note; //sfx.notes[note_idx].getKey();
        channel.current_note.n = sfx.notes[next_note_idx];
        channel.current_note.phi = channel.prev_note.phi;
        if (custom) {
            if (!sfx.notes[next_note_idx].getCustom() ||
                sfx.notes[next_note_idx].getKey() != sfx.notes[note_idx].getKey() ||
                sfx.notes[next_note_idx].getWaveform() != sfx.notes[note_idx].getWaveform()
              ) {
                channel.rotateChannels();
                channel.getChildChannel()->sfxId = -1;
            }
        }
    }
    return waveform;

}

float Audio::getSampleForNote(noteChannel &channel, rawSfxChannel &parentChannel, rawSfxChannel *childChannel, note prev_note, float freqShift, bool forceRemainder) {
    using std::max;
    float offset = parentChannel.offset;
    int const samples_per_second = 22050;
    //TODO: apply effects
    int const fx = channel.n.getEffect();
    uint8_t key = channel.n.getKey();
    float volume = channel.n.getVolume() / 7.f;
    float freq = key_to_freq(key);

    struct sfx const &sfx = _memory->sfx[parentChannel.sfxId];

    // Speed must be 1—255 otherwise the SFX is invalid
    int const speed = max(1, (int)sfx.speed);
    float const offset_per_second = 22050.f / (183.f * speed);
    int const note_idx = (int)floor(offset);

    // previous note effectively goes beyond offset fmod 1 when in crossfade
    float tmod= 0;
    if (forceRemainder) tmod = 1.0f;
    tmod += fmod(offset, 1.f);
    // Apply effect, if any
    switch (fx)
    {
        case FX_NO_EFFECT:
            break;
        case FX_SLIDE:
        {
            // From the documentation: “Slide to the next note and volume”,
            // but it’s actually _from_ the _prev_ note and volume.
            freq = lerp(key_to_freq(prev_note.getKey()), freq, tmod);
            if (prev_note.getVolume() > 0)
                volume = lerp(prev_note.getVolume() / 7.0f, volume, tmod);
            break;
        }
        case FX_VIBRATO:
        {
            // 7.5f and 0.25f were found empirically by matching
            // frequency graphs of PICO-8 instruments.
            float t = fabs(fmod(7.5f * tmod / offset_per_second, 1.0f) - 0.5f) - 0.25f;
            // Vibrato half a semi-tone, so multiply by pow(2,1/12)
            freq = lerp(freq, freq * 1.059463094359f, t);
            break;
        }
        case FX_DROP:
            freq *= 1.f - fmod(offset, 1.f);
            break;
        case FX_FADE_IN:
            volume *= std::min(1.f, tmod);
            break;
        case FX_FADE_OUT:
            volume *= max(0.0f, 1.f - tmod);
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
    freq*=freqShift;
    
    bool custom = (bool) channel.n.getCustom() && childChannel != NULL;
    float waveform;
    if (custom) {
      if (childChannel->sfxId == -1) {
        // initialize child channel
        childChannel->sfxId = channel.n.getWaveform();
        childChannel->offset = 0;
        childChannel->current_note.phi = 0;
        childChannel->can_loop = true;
        // don't want to double lower volume for music subchannel
        childChannel->is_music = false;
        childChannel->prev_note.n.setKey(0);
        childChannel->prev_note.n.setVolume(0);
      }
      waveform = volume * this->getSampleForSfx(*childChannel, freq/C2_FREQ);
    } else {
      waveform = volume * z8::synth::waveform(channel.n.getWaveform(), channel.phi);
    }
    channel.phi = channel.phi + freq / samples_per_second;
    return waveform;
}


//adapted from zepto8 sfx.cpp (wtfpl license)
int16_t Audio::getSampleForChannel(int channel){
    using std::fabs;
    using std::fmod;
    using std::floor;
    using std::max;

    int const samples_per_second = 22050;

    int16_t sample = 0;

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
        else if (_audioState._musicChannel.offset >= _audioState._musicChannel.length)
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

    sample = (int16_t) (32767.99f * this->getSampleForSfx(_audioState._sfxChannels[channel]));

    // TODO: Apply hardware effects
    if (_memory->hwState.distort & (1 << channel)) {
        sample = sample / 0x1000 * 0x1249;
    }
    return sample;
}

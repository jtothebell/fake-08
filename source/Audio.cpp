#include "Audio.h"
#include "synth.h"

#include <string>
#include <sstream>
#include <cmath>

//playback implemenation based on zetpo 8's
//https://github.com/samhocevar/zepto8/blob/master/src/pico8/sfx.cpp

Audio::Audio(){
    for(int i = 0; i < 4; i++) {
        _sfxChannels[i].sfxId = -1;
    }
}

void Audio::setMusic(std::string musicString){
    std::istringstream s(musicString);
    std::string line;
    char buf[3] = {0};
    int musicIdx = 0;
    
    while (std::getline(s, line)) {
        buf[0] = line[0];
        buf[1] = line[1];
        uint8_t flagByte = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[3];
        buf[1] = line[4];
        uint8_t channel1byte = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[5];
        buf[1] = line[6];
        uint8_t channel2byte = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[7];
        buf[1] = line[8];
        uint8_t channel3byte = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[9];
        buf[1] = line[10];
        uint8_t channel4byte = (uint8_t)strtol(buf, NULL, 16);

        _music[musicIdx++] = {
            flagByte,
            channel1byte,
            channel2byte,
            channel3byte,
            channel4byte
        };
    }

}

void Audio::setSfx(std::string sfxString) {
    std::istringstream s(sfxString);
    std::string line;
    char buf[3] = {0};
    int sfxIdx = 0;
    
    while (std::getline(s, line)) {
        buf[0] = line[0];
        buf[1] = line[1];
        uint8_t editorMode = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[2];
        buf[1] = line[3];
        uint8_t noteDuration = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[4];
        buf[1] = line[5];
        uint8_t loopRangeStart = (uint8_t)strtol(buf, NULL, 16);

        buf[0] = line[6];
        buf[1] = line[7];
        uint8_t loopRangeEnd = (uint8_t)strtol(buf, NULL, 16);

        _sfx[sfxIdx].editorMode = editorMode;
        _sfx[sfxIdx].speed = noteDuration;
        _sfx[sfxIdx].loopRangeStart = loopRangeStart;
        _sfx[sfxIdx].loopRangeEnd = loopRangeEnd;

        //32 notes, 5 chars each
        int noteIdx = 0;
        for (int i = 8; i < 168; i+=5) {
            buf[0] = line[i];
            buf[1] = line[i + 1];
            uint8_t pitch = (uint8_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 2];
            uint8_t waveform = (uint8_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 3];
            uint8_t volume = (uint8_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 4];
            uint8_t effect = (uint8_t)strtol(buf, NULL, 16);

            _sfx[sfxIdx++].notes[noteIdx] = {
                pitch,
                waveform,
                volume,
                effect
            };
        }

        sfxIdx++;
    } 
}

void Audio::api_sfx(uint8_t sfx, int channel, int offset){

    if (channel < 0 ||channel > 3) {
        //todo: handle these options later
        return;
    }

    this->_sfxChannels[channel].sfxId = sfx;
    this->_sfxChannels[channel].offset = std::max(0.f, (float)offset);
    this->_sfxChannels[channel].phi = 0.f;
    this->_sfxChannels[channel].can_loop = true;
    this->_sfxChannels[channel].is_music = false;
    this->_sfxChannels[channel].prev_key = 24;
    this->_sfxChannels[channel].prev_vol = 0.f;
        
}

void Audio::api_music(uint8_t pattern, int16_t fade_len, int16_t mask){
    if (pattern < 0 || pattern > 63) {
        return;
    }

    this->_musicChannel.count = 0;
    
    //todo: come back to this after sfx working

    this->_musicChannel.pattern = pattern;
    this->_musicChannel.offset = 0;

    
}



//todo: figure out where to store this
#define SAMPLERATE 22050
//----------------------------------------------------------------------------
void fill_buffer(void *audioBuffer,size_t offset, size_t size, int frequency ) {
//----------------------------------------------------------------------------

	uint32_t *dest = (uint32_t*)audioBuffer;

	for (size_t i=0; i<size; i++) {

		int16_t sample = INT16_MAX * sin(frequency*(2*M_PI)*(offset+i)/SAMPLERATE);

		dest[i] = (sample<<16) | (sample & 0xffff);
	}
}


void Audio::FillAudioBuffer(void *audioBuffer, size_t offset, size_t size){

    uint32_t *buffer = (uint32_t *)audioBuffer;

    for (size_t i = 0; i < size; ++i){
        int16_t sample = 0;

        for (int c = 0; c < 4; ++c) {
            sample += this->getSampleForChannel(c);
            //int16_t sample = INT16_MAX * sin(440*(2*M_PI)*(offset+i)/SAMPLERATE);

            
        }
        buffer[i] = (sample<<16) | (sample & 0xffff);
    }
}

static float key_to_freq(float key)
{
    using std::exp2;
    return 440.f * exp2((key - 33.f) / 12.f);
}



//adapted from zepto8 sfx.cpp (wtfpl license)
int16_t Audio::getSampleForChannel(int channel){
    using std::fabs, std::fmod, std::floor, std::max;

    int const samples_per_second = 22050;

    int16_t sample = 0;

    const int index = _sfxChannels[channel].sfxId;

    if (index < 0 || index > 63) {
        //no (valid) sfx here. return silence
        return 0;
    }

    struct sfx const &sfx = _sfx[index];

    // Speed must be 1—255 otherwise the SFX is invalid
    int const speed = max(1, (int)sfx.speed);

    float const offset = _sfxChannels[channel].offset;
    float const phi = _sfxChannels[channel].phi;

    // PICO-8 exports instruments as 22050 Hz WAV files with 183 samples
    // per speed unit per note, so this is how much we should advance
    float const offset_per_second = 22050.f / (183.f * speed);
    float const offset_per_sample = offset_per_second / samples_per_second;
    float next_offset = offset + offset_per_sample;

    // Handle SFX loops. From the documentation: “Looping is turned
    // off when the start index >= end index”.
    float const loop_range = float(sfx.loopRangeEnd - sfx.loopRangeStart);
    if (loop_range > 0.f && next_offset >= sfx.loopRangeStart && _sfxChannels[channel].can_loop) {
        next_offset = fmod(next_offset - sfx.loopRangeStart, loop_range)
                    + sfx.loopRangeStart;
    }

    int const note_id = (int)floor(offset);
    int const next_note_id = (int)floor(next_offset);

    uint8_t key = sfx.notes[note_id].pitch;
    float volume = sfx.notes[note_id].volume / 7.f;
    float freq = key_to_freq(key);

    if (volume == 0.f){
        //volume all the way off. return silence, but make sure to set stuff
        _sfxChannels[channel].offset = next_offset;

        if (next_offset >= 32.f){
            _sfxChannels[channel].sfxId = -1;
        }
        else if (next_note_id != note_id){
            _sfxChannels[channel].prev_key = sfx.notes[note_id].pitch;
            _sfxChannels[channel].prev_vol = sfx.notes[note_id].volume / 7.f;
        }

        return 0;
    }
    
    //TODO: apply effects
    //int const fx = sfx.notes[note_id].effect;

    

    // Play note

    float waveform = z8::synth::waveform(sfx.notes[note_id].waveform, phi);

    // Apply master music volume from fade in/out
    // FIXME: check whether this should be done after distortion
    //if (m_state.channels[chan].is_music) {
    //    volume *= m_state.music.volume;
    //}

    sample = (int16_t)(32767.99f * volume * waveform);

    // TODO: Apply hardware effects
    //if (m_ram.hw_state.distort & (1 << chan)) {
    //    sample = sample / 0x1000 * 0x1249;
    //}

    _sfxChannels[channel].phi = phi + freq / samples_per_second;

    _sfxChannels[channel].offset = next_offset;

    if (next_offset >= 32.f){
        _sfxChannels[channel].sfxId = -1;
    }
    else if (next_note_id != note_id){
        _sfxChannels[channel].prev_key = sfx.notes[note_id].pitch;
        _sfxChannels[channel].prev_vol = sfx.notes[note_id].volume / 7.f;
    }

    return sample;
}

#include "Audio.h"

#include <string>
#include <sstream>

//playback implemenation based on zetpo 8's
//https://github.com/samhocevar/zepto8/blob/master/src/pico8/sfx.cpp

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

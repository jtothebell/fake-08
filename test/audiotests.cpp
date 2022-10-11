#include "doctest.h"
#include "../source/Audio.h"
#include "../source/PicoRam.h"

TEST_CASE("audio class behaves as expected") {
    //general setup
    PicoRam picoRam;
    picoRam.Reset();
    Audio* audio = new Audio(&picoRam);
    audioState_t* audioState = audio->getAudioState();

    SUBCASE("Audio constructor sets sfx channels to -1") {
        bool allChannelsOff = true;
        
        for(int i = 0; i < 4; i++) {
            allChannelsOff &= audioState->_sfxChannels[i].sfxId == -1;
        }

        CHECK(allChannelsOff);
    }
    SUBCASE("api_sfx() with valid values sets the sfx to be played") {
        int channel = 0;
        int sfxId = 3;
        audio->api_sfx(sfxId, channel, 0);

        CHECK_EQ(audioState->_sfxChannels[0].sfxId, sfxId);
    }
    SUBCASE("api_sfx() with -1 channel finds first available") {
        audio->api_sfx(1, 0, 0);
        audio->api_sfx(2, 1, 0);
        audio->api_sfx(5, -1, 0);


        CHECK_EQ(audioState->_sfxChannels[2].sfxId, 5);
    }
    SUBCASE("api_sfx() -2 sfx id stops looping") {
        audioState->_sfxChannels[3].can_loop = true;

        audio->api_sfx(-2, 3, 0);

        CHECK_EQ(audioState->_sfxChannels[3].can_loop, false);
    }
    SUBCASE("api_sfx() -2 sfx id stops looping") {
        audioState->_sfxChannels[3].can_loop = true;

        audio->api_sfx(-2, 3, 0);

        CHECK_EQ(audioState->_sfxChannels[3].can_loop, false);
    }
    SUBCASE("api_music sets music pattern"){
        audio->api_music(14, 0, 0);

        CHECK_EQ(audioState->_musicChannel.pattern, 14);
    }
    SUBCASE("api_music sets sfx channels"){
        picoRam.songs[3].data[0]=9;
        picoRam.songs[3].data[1]=10;
        picoRam.songs[3].data[2]=11;
        picoRam.songs[3].data[3]=12;
        audio->api_music(3, 0, 0);
        CHECK_EQ(audioState->_sfxChannels[0].sfxId, 9);
        CHECK_EQ(audioState->_sfxChannels[1].sfxId, 10);
        CHECK_EQ(audioState->_sfxChannels[2].sfxId, 11);
        CHECK_EQ(audioState->_sfxChannels[3].sfxId, 12);
    }

    SUBCASE("api_music makes master fastest sfx channels"){
        picoRam.songs[3].data[0]=0;
        picoRam.songs[3].data[1]=1;
        picoRam.songs[3].data[2]=2;
        picoRam.songs[3].data[3]=3;
        picoRam.sfx[0].speed = 4;
        picoRam.sfx[1].speed = 6;
        picoRam.sfx[2].speed = 3;
        picoRam.sfx[3].speed = 5;
        audio->api_music(3, 0, 0);
        CHECK_EQ(audioState->_sfxChannels[audioState->_musicChannel.master].sfxId, 2);
    }

    SUBCASE("api_music makes master shortest sfx channels"){
        picoRam.songs[3].data[0]=0;
        picoRam.songs[3].data[1]=1;
        picoRam.songs[3].data[2]=2;
        picoRam.songs[3].data[3]=3;
        picoRam.sfx[0].loopRangeStart = 9;
        picoRam.sfx[1].loopRangeStart = 6;
        picoRam.sfx[2].loopRangeStart = 30;
        picoRam.sfx[3].loopRangeStart = 7;
        audio->api_music(3, 0, 0);
        CHECK_EQ(audioState->_sfxChannels[audioState->_musicChannel.master].sfxId, 1);
    }

    SUBCASE("api_music makes master shortest/fastest sfx channels"){
        picoRam.songs[3].data[0]=0;
        picoRam.songs[3].data[1]=1;
        picoRam.songs[3].data[2]=2;
        picoRam.songs[3].data[3]=3;
        picoRam.sfx[0].loopRangeStart = 9;
        picoRam.sfx[1].loopRangeStart = 6;
        picoRam.sfx[2].loopRangeStart = 30;
        picoRam.sfx[3].loopRangeStart = 7;
        picoRam.sfx[0].speed = 4;
        picoRam.sfx[1].speed = 6;
        picoRam.sfx[2].speed = 3;
        picoRam.sfx[3].speed = 5;
        audio->api_music(3, 0, 0);
        CHECK_EQ(audioState->_sfxChannels[audioState->_musicChannel.master].sfxId, 3);
    }

    SUBCASE("api_music prefers non-looping"){
        picoRam.songs[3].data[0]=0;
        picoRam.songs[3].data[1]=1;
        picoRam.songs[3].data[2]=2;
        picoRam.songs[3].data[3]=3;
        picoRam.sfx[0].loopRangeEnd = 9;
        picoRam.sfx[0].speed = 4;
        picoRam.sfx[1].speed = 6;
        picoRam.sfx[2].speed = 9;
        picoRam.sfx[3].speed = 99;
        audio->api_music(3, 0, 0);
        CHECK_EQ(audioState->_sfxChannels[audioState->_musicChannel.master].sfxId, 1);
    }

    SUBCASE("api_music picks fastest looping if all looping ignoring loop length"){
        picoRam.songs[3].data[0]=0;
        picoRam.songs[3].data[1]=1;
        picoRam.songs[3].data[2]=2;
        picoRam.songs[3].data[3]=3;
        picoRam.sfx[0].loopRangeEnd = 9;
        picoRam.sfx[1].loopRangeEnd = 6;
        picoRam.sfx[2].loopRangeEnd = 30;
        picoRam.sfx[3].loopRangeEnd = 7;
        picoRam.sfx[0].speed = 4;
        picoRam.sfx[1].speed = 6;
        picoRam.sfx[2].speed = 3;
        picoRam.sfx[3].speed = 5;
        audio->api_music(3, 0, 0);
        CHECK_EQ(audioState->_sfxChannels[audioState->_musicChannel.master].sfxId, 2);
    }
}

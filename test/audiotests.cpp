#include "doctest.h"
#include "../source/Audio.h"
#include "../source/PicoRam.h"

TEST_CASE("audio class behaves as expected") {
    //general setup
    PicoRam picoRam;
    picoRam = {0};
    Audio* audio = new Audio(&picoRam);

    SUBCASE("Audio constructor sets sfx channels to -1") {
        bool allChannelsOff = true;
        
        for(int i = 0; i < 4; i++) {
            allChannelsOff &= picoRam._sfxChannels[i].sfxId == -1;
        }

        CHECK(allChannelsOff);
    }
    SUBCASE("api_sfx() with valid values sets the sfx to be played") {
        int channel = 0;
        int sfxId = 3;
        audio->api_sfx(sfxId, channel, 0);

        CHECK_EQ(picoRam._sfxChannels[0].sfxId, sfxId);
    }

}
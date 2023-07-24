#include "doctest.h"
#include "../source/Audio.h"
#include "../source/PicoRam.h"
#include "../source/cart.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

typedef struct WAV_HEADER {
  /* RIFF Chunk Descriptor */
  uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
  uint32_t ChunkSize;                     // RIFF Chunk Size
  uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
  /* "fmt" sub-chunk */
  uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
  uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
  uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
                            // Mu-Law, 258=IBM A-Law, 259=ADPCM
  uint16_t NumOfChan = 1;   // Number of channels 1=Mono 2=Sterio
  uint32_t SamplesPerSec = 22050;   // Sampling Frequency in Hz
  uint32_t bytesPerSec = 22050 * 2; // bytes per second
  uint16_t blockAlign = 2;          // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample = 16;      // Number of bits per sample
  /* "data" sub-chunk */
  uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
  uint32_t Subchunk2Size;                        // Sampled data length
} wav_hdr;

void write_wav(std::istream *in, const char *output_filename) {
  static_assert(sizeof(wav_hdr) == 44, "");

  std::string in_name = "test.bin"; // raw pcm data without wave header

  uint32_t fsize = in->tellg();
  in->seekg(0, std::ios::end);
  fsize = (uint32_t)in->tellg() - fsize;
  in->seekg(0, std::ios::beg);

  wav_hdr wav;
  wav.ChunkSize = fsize + sizeof(wav_hdr) - 8;
  wav.Subchunk2Size = fsize + sizeof(wav_hdr) - 44;

  std::ofstream out(output_filename, std::ios::binary);
  out.write(reinterpret_cast<const char *>(&wav), sizeof(wav));

  int16_t d;
  for (int i = 0; i < fsize; ++i) {
    // TODO: read/write in blocks
    in->read(reinterpret_cast<char *>(&d), sizeof(int16_t));
    out.write(reinterpret_cast<char *>(&d), sizeof(int16_t));
  }
}

TEST_CASE("audio class behaves as expected") {
    //general setup
    PicoRam picoRam;
    picoRam.Reset();
    Audio* audio = new Audio(&picoRam);
    audioState_t* audioState = audio->getAudioState();

   SUBCASE("custom instruments same note"){
        note c;

        c.setCustom(true);
        c.setWaveform(1);
        c.setVolume(5);
        c.setKey(24);

        picoRam.sfx[0].speed = 32;
        picoRam.sfx[0].notes[0] = c;
        picoRam.sfx[0].notes[1] = c;

        picoRam.sfx[1].speed=8;
        //183 is one tick
        for (int i=0;i<8;i++) {
          picoRam.sfx[1].notes[i].setVolume(5);
          picoRam.sfx[1].notes[i].setWaveform(1);
        }
        picoRam.sfx[1].notes[0].setKey(24);
        picoRam.sfx[1].notes[1].setKey(26);
        picoRam.sfx[1].notes[2].setKey(28);
        picoRam.sfx[1].notes[3].setKey(29);
        picoRam.sfx[1].notes[4].setKey(31);
        picoRam.sfx[1].notes[5].setKey(33);
        picoRam.sfx[1].notes[6].setKey(35);
        picoRam.sfx[1].notes[7].setKey(36);

        sfxChannel s;
        s.sfxId=0;
        s.offset=0;
        s.current_note.phi=0;

        for (int i=0; i<183*4; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 24);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 26);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 28);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 29);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 31);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 33);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 35);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 36);
   }

   SUBCASE("custom instruments different note"){
        note c,d;

        c.setCustom(true);
        c.setWaveform(1);
        c.setVolume(5);
        c.setKey(24);
        d.setCustom(true);
        d.setWaveform(1);
        d.setVolume(5);
        d.setKey(26);

        picoRam.sfx[0].speed = 32;
        picoRam.sfx[0].notes[0] = c;
        picoRam.sfx[0].notes[1] = d;

        picoRam.sfx[1].speed=8;
        //183 is one tick
        for (int i=0;i<8;i++) {
          picoRam.sfx[1].notes[i].setVolume(5);
          picoRam.sfx[1].notes[i].setWaveform(1);
        }
        picoRam.sfx[1].notes[0].setKey(24);
        picoRam.sfx[1].notes[1].setKey(26);
        picoRam.sfx[1].notes[2].setKey(28);
        picoRam.sfx[1].notes[3].setKey(29);
        picoRam.sfx[1].notes[4].setKey(31);
        picoRam.sfx[1].notes[5].setKey(33);
        picoRam.sfx[1].notes[6].setKey(35);
        picoRam.sfx[1].notes[7].setKey(36);

        sfxChannel s;
        s.sfxId=0;
        s.offset=0;
        s.current_note.phi=0;

        for (int i=0; i<183*4; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 24);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 26);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 28);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 29);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 24);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 26);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 28);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
        CHECK_EQ(s.getChildChannel()->current_note.n.getKey(), 29);
        for (int i=0; i<183*8; i++) { audio->getSampleForSfx(s); }
   }

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
    SUBCASE("api_sfx() with -2 channel stops the sfx on any channel") {
        audio->api_sfx(1, 0, 0);
        audio->api_sfx(10, 1, 0);
        audio->api_sfx(20, 2, 0);
        audio->api_sfx(1, 3, 0);
        audio->api_sfx(1, -2, 0);


        CHECK_EQ(audioState->_sfxChannels[0].sfxId, -1);
        CHECK_EQ(audioState->_sfxChannels[1].sfxId, 10);
        CHECK_EQ(audioState->_sfxChannels[2].sfxId, 20);
        CHECK_EQ(audioState->_sfxChannels[3].sfxId, -1);
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

    SUBCASE("bass loop effect"){
      Cart* cart = new Cart("songtest.p8", "carts");
      memcpy(&picoRam.data[0], &cart->CartRom.data[0], sizeof(cart->CartRom));
      audio->api_music(41, 0, 0xf);

      // get 1 second
      std::stringbuf buffer;
      std::iostream os (&buffer);  // associate stream buffer to stream

      for (int i=1; i<22050*20; i++) {
        int16_t sample = audio->getSampleForChannel(0);
        os.write(reinterpret_cast<char *>(&sample), sizeof(int16_t));
      }
      write_wav(&os, "bass.wav");
      delete cart;
    }

}

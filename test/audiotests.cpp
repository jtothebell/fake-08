#include "doctest.h"
#include "../source/Audio.h"
#include "../source/PicoRam.h"
#include "../source/cart.h"
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

TEST_CASE("vibrato produces triangle wave modulation") {
    PicoRam picoRam;
    picoRam.Reset();
    Audio* audio = new Audio(&picoRam);

    // Set up SFX 0: single note with vibrato effect, square wave for easy analysis
    // Pitch 24 (C-2), waveform 3 (square), volume 7, effect 2 (vibrato)
    for (int n = 0; n < 32; n++) {
        picoRam.sfx[0].notes[n].setKey(24);
        picoRam.sfx[0].notes[n].setWaveform(3);
        picoRam.sfx[0].notes[n].setVolume(7);
        picoRam.sfx[0].notes[n].setEffect(FX_VIBRATO);
    }
    picoRam.sfx[0].speed = 16;

    audio->api_sfx(0, 0, 0, 31);

    // Generate 1 second of audio (22050 samples)
    const int num_samples = 22050;
    int16_t samples[num_samples];
    audio->FillMonoAudioBuffer(samples, 0, num_samples);

    // Measure zero-crossing intervals to detect pitch modulation shape.
    // Vibrato modulates the pitch, so zero-crossing intervals vary over
    // time. With correct triangle modulation, intervals change smoothly.
    // With the sawtooth bug, intervals jump abruptly at cycle boundaries.
    //
    // Collect zero-crossing intervals and check that consecutive intervals
    // never jump by more than a threshold (smooth modulation).
    std::vector<int> zc_intervals;
    int last_zc = -1;
    for (int i = 1; i < num_samples; i++) {
        if ((samples[i-1] <= 0 && samples[i] > 0)) {
            if (last_zc >= 0) {
                zc_intervals.push_back(i - last_zc);
            }
            last_zc = i;
        }
    }

    // With vibrato at C-2 (~261 Hz), we expect ~261 zero crossings/sec,
    // so ~261 intervals in 1 second. Check we got a reasonable number.
    CHECK(zc_intervals.size() > 100);

    // Find the maximum jump between consecutive zero-crossing intervals.
    // Triangle modulation: intervals change by ~1 sample between crossings.
    // Sawtooth bug: intervals jump by several samples at the discontinuity.
    int max_interval_jump = 0;
    for (size_t i = 1; i < zc_intervals.size(); i++) {
        int jump = std::abs(zc_intervals[i] - zc_intervals[i-1]);
        if (jump > max_interval_jump) max_interval_jump = jump;
    }

    // With triangle vibrato, max jump should be small (<=4 samples).
    // With sawtooth bug, jumps of 8+ samples occur at cycle boundaries.
    CHECK(max_interval_jump <= 5);

    // Also verify the signal is non-silent (vibrato didn't kill the output)
    float rms = 0;
    for (int i = 0; i < num_samples; i++) {
        rms += (float)samples[i] * samples[i];
    }
    rms = std::sqrt(rms / num_samples);
    CHECK(rms > 100.0f);

    delete audio;
}

TEST_CASE("audio class behaves as expected") {
    //general setup
    PicoRam picoRam;
    picoRam.Reset();
    Audio* audio = new Audio(&picoRam);
    audioState_t* audioState = audio->getAudioState();

    // Note: The custom instrument tests have been removed as the new audio
    // implementation no longer exposes getSampleForSfx as a public method
    // and the internal structure has changed significantly.

    SUBCASE("Audio constructor sets sfx channels to -1") {
        bool allChannelsOff = true;
        
        for(int i = 0; i < 4; i++) {
            allChannelsOff &= audioState->_sfxChannels[i].main_sfx.sfx == -1;
        }

        CHECK(allChannelsOff);
    }
    SUBCASE("api_sfx() with valid values sets the sfx to be played") {
        int channel = 0;
        int sfxId = 3;
        audio->api_sfx(sfxId, channel, 0, 31);

        CHECK_EQ(audioState->_sfxChannels[0].main_sfx.sfx, sfxId);
    }
    SUBCASE("api_sfx() with -1 channel finds first available") {
        audio->api_sfx(1, 0, 0, 31);
        audio->api_sfx(2, 1, 0, 31);
        audio->api_sfx(5, -1, 0, 31);


        CHECK_EQ(audioState->_sfxChannels[2].main_sfx.sfx, 5);
    }
    SUBCASE("api_sfx() with -2 channel stops the sfx on any channel") {
        audio->api_sfx(1, 0, 0, 31);
        audio->api_sfx(10, 1, 0, 31);
        audio->api_sfx(20, 2, 0, 31);
        audio->api_sfx(1, 3, 0, 31);
        audio->api_sfx(1, -2, 0, 31);


        CHECK_EQ(audioState->_sfxChannels[0].main_sfx.sfx, -1);
        CHECK_EQ(audioState->_sfxChannels[1].main_sfx.sfx, 10);
        CHECK_EQ(audioState->_sfxChannels[2].main_sfx.sfx, 20);
        CHECK_EQ(audioState->_sfxChannels[3].main_sfx.sfx, -1);
    }
    SUBCASE("api_sfx() -2 sfx id stops looping") {
        audioState->_sfxChannels[3].can_loop = true;

        audio->api_sfx(-2, 3, 0, 31);

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
        CHECK_EQ(audioState->_sfxChannels[0].main_sfx.sfx, 9);
        CHECK_EQ(audioState->_sfxChannels[1].main_sfx.sfx, 10);
        CHECK_EQ(audioState->_sfxChannels[2].main_sfx.sfx, 11);
        CHECK_EQ(audioState->_sfxChannels[3].main_sfx.sfx, 12);
    }

    // Note: The master channel tests have been removed as the new implementation
    // no longer uses a master field. The music system now uses pattern-based
    // length calculation instead of tracking a master channel.

    SUBCASE("bass loop effect"){
      Cart* cart = new Cart("songtest.p8", "carts");
      memcpy(&picoRam.data[0], &cart->CartRom.data[0], sizeof(cart->CartRom));
      audio->api_music(41, 0, 0xf);

      // get 20 seconds of audio
      std::stringbuf buffer;
      std::iostream os (&buffer);  // associate stream buffer to stream

      int16_t samples[22050];
      for (int chunk = 0; chunk < 20; chunk++) {
        audio->FillMonoAudioBuffer(samples, 0, 22050);
        for (int i = 0; i < 22050; i++) {
          os.write(reinterpret_cast<char *>(&samples[i]), sizeof(int16_t));
        }
      }
      write_wav(&os, "bass.wav");
      delete cart;
    }

}

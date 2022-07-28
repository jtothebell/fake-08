#include <stdio.h>
#include <string.h>
#include <array>

#include "libretro.h"

#include "../../source/vm.h"
#include "../../source/PicoRam.h"
#include "../../source/Audio.h"
#include "../../source/host.h"
#include "../../source/hostVmShared.h"
#include "../../source/nibblehelpers.h"
#include "setInput.h"


//since this is a C api, we need to mark all these as extern
//so the compiler doesn't mangle the symbol names
#define EXPORT extern "C" RETRO_API

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t enviro_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;



#define SAMPLERATE 22050
#define SAMPLESPERBUF (SAMPLERATE / 60)
//#define SAMPLESPERBUF 1024
#define NUM_BUFFERS 2
const size_t audioBufferSize = SAMPLESPERBUF * NUM_BUFFERS;

int16_t audioBuffer[audioBufferSize];

const int PicoScreenWidth = 128;
const int PicoScreenHeight = 128;

const int BytesPerPixel = 2;

const size_t screenBufferSize = PicoScreenWidth*PicoScreenHeight;

uint16_t screenBuffer[screenBufferSize];

uint16_t _rgb565Colors[144];

Vm* _vm;
PicoRam* _memory;
Audio* _audio;
Host* _host;

double prev_frame_time = 0;
double frame_time = 0;

static void frame_time_cb(retro_usec_t usec)
{
    prev_frame_time = frame_time;
    frame_time = usec / 1000000.0;
}


EXPORT void retro_set_environment(retro_environment_t cb)
{
    //allow to run built in bios if no rom is passed
    enviro_cb = cb;
    bool no_rom = true;
    enviro_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_rom);

    //this came from another stub... not sure if we need it?
    char const *system_dir;
    enviro_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir);

    struct retro_frame_time_callback frame_cb = { frame_time_cb, 1000000 / 60 };
    enviro_cb(RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK, &frame_cb);
}

EXPORT void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
EXPORT void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }
EXPORT void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
EXPORT void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
EXPORT void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

EXPORT void retro_init()
{
    //called once. do setup (create host and vm?)
    _host = new Host();
	_memory = new PicoRam();
	_audio = new Audio(_memory);

    _vm = new Vm(_host, _memory, nullptr, nullptr, _audio);

    _host->setUpPaletteColors();
    Color* paletteColors = _host->GetPaletteColors();

	_host->oneTimeSetup(_audio);

    for(int i = 0; i < 144; i++){
        _rgb565Colors[i] = (((paletteColors[i].Red & 0xf8)<<8) + ((paletteColors[i].Green & 0xfc)<<3)+(paletteColors[i].Blue>>3));
    }

    _vm->SetCartList(_host->listcarts());

    _vm->LoadBiosCart();
}

EXPORT void retro_deinit()
{
    //delete things created in init
    _vm->CloseCart();
    _host->oneTimeCleanup();
    delete _vm;
    delete _host;
}

EXPORT unsigned retro_api_version()
{
   return RETRO_API_VERSION;
}

EXPORT void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "fake-08";
    info->library_version = "0.0.2.18"; //todo: get from build flags
    info->valid_extensions = "p8|p8.png";
    info->need_fullpath = true; // we load our own carts for now
}

EXPORT void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info, 0, sizeof(*info));
    info->geometry.base_width = PicoScreenWidth;
    info->geometry.base_height = PicoScreenHeight;
    info->geometry.max_width = PicoScreenWidth;
    info->geometry.max_height = PicoScreenHeight;
    info->geometry.aspect_ratio = 1.f;
    info->timing.fps = 60.f; //todo: update this to 60, then handle 30 at callback level?
    info->timing.sample_rate = 22050.f;

    retro_pixel_format pf = RETRO_PIXEL_FORMAT_RGB565;
    enviro_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pf);
}

EXPORT void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

EXPORT void retro_reset()
{
}

static std::array<int, 7> buttons
{
    RETRO_DEVICE_ID_JOYPAD_LEFT,
    RETRO_DEVICE_ID_JOYPAD_RIGHT,
    RETRO_DEVICE_ID_JOYPAD_UP,
    RETRO_DEVICE_ID_JOYPAD_DOWN,
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_START,
};

uint8_t kHeld = 0;
uint8_t kDown = 0;

size_t frame = 0;

EXPORT void retro_run()
{
    //TODO: improve this so slower hardware can play 30fps games at full speed
    if (_vm->getTargetFps() == 60 || frame % 2 == 0)
    {
        input_poll_cb();

        uint8_t currKDown = 0;
        uint8_t currKHeld = 0;
        for (int i = 0; i < 7; i++) {
            bool down = input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, buttons[i]);
            if (down) {
                currKHeld |= BITMASK(i);
                //if the key is a new push this frame, mark it as down as well
                if (!(kHeld & BITMASK(i))) {
                    currKDown |= BITMASK(i);
                }
            }
        }

        setInputState(currKDown, currKHeld);

        _vm->UpdateAndDraw();
        kHeld = currKHeld;
        kDown = currKDown;

        if (frame % 2 == 0) {
            _audio->FillAudioBuffer(&audioBuffer, 0, audioBufferSize);
            audio_batch_cb(audioBuffer, SAMPLESPERBUF * NUM_BUFFERS);
        }

    }

    

    uint8_t* picoFb = _vm->GetPicoInteralFb();
    uint8_t* screenPaletteMap = _vm->GetScreenPaletteMap();

    for (size_t pixIdx = 0; pixIdx < screenBufferSize; pixIdx++){
        screenBuffer[pixIdx] = _rgb565Colors[screenPaletteMap[getPixelNibble(pixIdx % 128, pixIdx / 128, picoFb)]];
    }

    video_cb(&screenBuffer, PicoScreenWidth, PicoScreenHeight, PicoScreenWidth * BytesPerPixel);

    frame++;
}

EXPORT size_t retro_serialize_size()
{
    return 0;
}

EXPORT bool retro_serialize(void *data, size_t size)
{
    return false;
}

EXPORT bool retro_unserialize(const void *data, size_t size)
{
    return false;
}

EXPORT void retro_cheat_reset()
{
}

EXPORT void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
}

EXPORT bool retro_load_game(struct retro_game_info const *info)
{
    _vm->QueueCartChange(info->path);
    return true;
}

EXPORT bool retro_load_game_special(unsigned game_type,
    const struct retro_game_info *info, size_t num_info)
{
    return false;
}

EXPORT void retro_unload_game()
{
    _vm->CloseCart();
}

EXPORT unsigned retro_get_region()
{
    return 0;
}

EXPORT void *retro_get_memory_data(unsigned id)
{
    return nullptr;
}

EXPORT size_t retro_get_memory_size(unsigned id)
{
    return 0;
}
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <stack>
#include <array>
#include <algorithm>

//make sure 3ds and switch libpng are installed from devkitpro pacman
//#include <png.h>
#include "lodepng.h"

#include "cart.h"
#include "filehelpers.h"

#include "stringToDataHelpers.h"

#include "utils.h"

#include "logger.h"

#include "cartPatcher.h"
#include "emojiconversion.h"

#include "FakoBios.h"

//#include "tests/test_base.h"
//#if _TEST
//#include "tests/cart_test.h"
//#endif

static char const *legacyCompressionLut = "\n 0123456789abcdefghijklmnopqrstuvwxyz!#%(){}[]<>+=/*:;.,~_";


//https://github.com/samhocevar/zepto8/blob/b1a13516945c49e47495c739e6a43a241ad99291/src/pico8/code.cpp
// Move to front structure
struct move_to_front
{
    move_to_front()
    {
        reset();
    }

    void reset()
    {
        for (int n = 0; n < 256; ++n)
            state[n] = uint8_t(n);
    }

    // Get the nth byte and move it to front
    uint8_t get(int n)
    {
        std::rotate(state.begin(), state.begin() + n, state.begin() + n + 1);
        return state.front();
    }

    // Find index of a given character
    int find(uint8_t ch)
    {
        auto val = std::find(state.begin(), state.end(), ch);
        return int(std::distance(state.begin(), val));
    }

    // Push a character and return its previous index, allowing the caller to compute the cost
    // of the operation. This operation can be undone by pop_op().
    int push_op(uint8_t ch)
    {
        int n = find(ch);
        get(n);
        ops.push(uint8_t(n));
        return n;
    }

    // Undo an push_op() operation
    void pop_op()
    {
        std::rotate(state.begin(), state.begin() + 1, state.begin() + ops.top() + 1);
        ops.pop();
    }

private:
    std::array<uint8_t, 256> state;
    std::stack<uint8_t> ops;
};

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

#define HEADERLEN 8


bool Cart::loadCartFromPng(std::string filename){
    std::vector<unsigned char> image; //the raw pixels
    unsigned width, height;

    //decode
    unsigned error = lodepng::decode(image, width, height, filename);
    //the pixels are now in the vector "image", 4 bytes per pixel, ordered RGBARGBA..., use it as texture, draw it,

    //if there's an error, display it
    if(error) {
        LoadError = "png decoder error " + std::string(lodepng_error_text(error));
        Logger::Write("%s%s", LoadError.c_str(), "\n");
        return false;
    }

    if (width != 160 || height != 205) {
        LoadError = "Invalid png dimensions";
        Logger::Write("invalid dimensions\n");
        return false;
    }

    //160x205 == 32800 == 0x8020
    //0x8000 is actual used data size
    size_t imageBytes = image.size();

    uint8_t version = 0;

    for(size_t i = 0; i < imageBytes; i += 4) {
        //get argb values
        uint8_t r = image[i];
        uint8_t g = image[i + 1];
        uint8_t b = image[i + 2];
        uint8_t a = image[i + 3];

        //get lower bits where pico data is encoded
        a = a & 0x0003;
        r = r & 0x0003;
		g = g & 0x0003;
		b = b & 0x0003;

        uint8_t extractedByte = (a << 6) + (r << 4) + (g << 2) + b;

        //TODO: write this all to one array, then memcpy to picoram?
        //store extracted byte in correct place
        size_t picoDataIdx = i / 4;
        if (picoDataIdx < 0x2000) {
            SpriteSheetData[picoDataIdx] = extractedByte;
        }
        else if (picoDataIdx < 0x3000) {
            MapData[picoDataIdx - 0x2000] = extractedByte;
        }
        else if (picoDataIdx < 0x3100) {
            SpriteFlagsData[picoDataIdx - 0x3000] = extractedByte;
        }
        else if (picoDataIdx < 0x3200) {
            size_t offset = picoDataIdx - 0x3100;
            size_t songIdx = offset / sizeof(song);
            size_t channelIdx = offset % sizeof(song);
            SongData[songIdx].data[channelIdx] = extractedByte;
        }
        else if (picoDataIdx < 0x4300) {
            size_t offset = picoDataIdx - 0x3200;
            size_t sfxIdx = offset / sizeof(sfx);
            size_t byteIdx = offset % sizeof(sfx);
            SfxData[sfxIdx].data[byteIdx] = extractedByte;
        }
        else if (picoDataIdx < 0x8000) {
            CartLuaData[picoDataIdx - 0x4300] = extractedByte;
        }
        else if (picoDataIdx == 0x8000) {
            version = extractedByte;
        }
    }

    uint8_t compression = 0;

    if (CartLuaData[0] == '\0' && CartLuaData[1] == 'p' && CartLuaData[2] == 'x' && CartLuaData[3] == 'a'){
        compression = 2;
    }

    if (CartLuaData[0] == ':' && CartLuaData[1] == 'c' && CartLuaData[2] == ':' && CartLuaData[3] == '\0'){
        compression = 1;
    }

    Logger::Write("Version: %d Compression %d", version, compression);

    if (compression == 0) {
        auto codeBlockLength = 0x8000 - 0x4300;
        auto length = codeBlockLength;

        auto endOfCodePtr = (uint8_t const *)std::memchr(CartLuaData, '\0', codeBlockLength);
        if (endOfCodePtr) {
            length = endOfCodePtr - CartLuaData;
        }

        LuaString = std::string((char const *)CartLuaData, length);
    }
    else if (compression == 1){
        //from pico 8 wiki: https://pico-8.fandom.com/wiki/P8PNGFileFormat
        //The first four bytes (0x4300-0x4303) are :c:\x00.
        //The next two bytes (0x4304-0x4305) are the length of the decompressed code, stored MSB first.
        //The next two bytes (0x4306-0x4307) are always zero. 
        size_t length = CartLuaData[4] * 256 + CartLuaData[5];

        LuaString.resize(0);

        for (size_t i = 8; i < sizeof(CartLuaData) && LuaString.length() < length; ++i){
            //0x00: Copy the next byte directly to the output stream. 
            if(CartLuaData[i] == 0x00){
                LuaString += CartLuaData[++i];
            }
            //0x01-0x3b: Emit a character from a lookup table
            else if (CartLuaData[i] < 0x3c){
                LuaString += legacyCompressionLut[CartLuaData[i] - 1];
            }
            //0x3c-0xff: Calculate an offset and length from this byte and the next byte, 
            //then copy those bytes from what has already been emitted. In other words, 
            //go back "offset" characters in the output stream, copy "length" characters, 
            //then paste them to the end of the output stream. Offset and length are 
            //calculated as: 
            //   offset = (current_byte - 0x3c) * 16 + (next_byte & 0xf)
            //   length = (next_byte >> 4) + 2
            else {
                size_t offset = (CartLuaData[i] - 0x3c) * 16 + (CartLuaData[i + 1] & 0xf);
                size_t length = (CartLuaData[i + 1] >> 4) + 2;

                int startIndex = LuaString.length() - offset;
                if (startIndex > -1) {
                    for(size_t j = 0; j < length; ++j) {
                        LuaString += LuaString[startIndex + j];
                    }
                }

                ++i;
            }
        }

    }
    else if (compression == 2){
        //implementation from zepto8 code.cpp, pxa_decompress
        //https://github.com/samhocevar/zepto8/blob/b1a13516945c49e47495c739e6a43a241ad99291/src/pico8/code.cpp
        size_t length = CartLuaData[4] * 256 + CartLuaData[5];
        size_t compressed = CartLuaData[6] * 256 + CartLuaData[7];

        size_t pos = size_t(8) * 8; // stream position in bits
        auto get_bits = [&](size_t count) -> uint32_t
        {
            uint32_t n = 0;
            for (size_t i = 0; i < count; ++i, ++pos)
                n |= ((CartLuaData[pos >> 3] >> (pos & 0x7)) & 0x1) << i;
            return n;
        };

        move_to_front mtf;

        //Logger::Write("# Size: %d (%04x)\n", int(compressed), int(compressed));

        while (LuaString.size() < length && pos < compressed * 8)
        {
            auto oldpos = pos; (void)oldpos;

            if (get_bits(1))
            {
                int nbits = 4;
                while (get_bits(1))
                    ++nbits;
                int n = get_bits(nbits) + (1 << nbits) - 16;
                uint8_t ch = mtf.get(n);
                if (!ch)
                    break;
                //Logger::Write("%04x [%d] $%d\n", int(LuaString.size()), int(pos-oldpos), ch);
                LuaString.push_back(char(ch));
            }
            else
            {
                int nbits = get_bits(1) ? get_bits(1) ? 5 : 10 : 15;
                int offset = get_bits(nbits) + 1;

                int n, len = 3;
                do
                    len += (n = get_bits(3));
                while (n == 7);

                //Logger::Write("%04x [%d] %d@-%d\n", int(LuaString.size()), int(pos-oldpos), len, offset);
                for (int i = 0; i < len; ++i)
                    LuaString.push_back(LuaString[LuaString.size() - offset]);
            }
        }

    }    

    return true;

}

//tac08 based cart parsing and stripping of emoji
Cart::Cart(std::string filename){
    Filename = filename;
    //zero out cart rom so no garbage is left over
    initCartRom();

    Logger::Write("getting file contents\n");
    
    if (hasEnding(filename, ".p8")){
        std::string cartStr; 

        if (filename == "__FAKE08-BIOS.p8") {
            cartStr = fake08BiosP8;
        }
        else {
            cartStr = get_file_contents(filename.c_str());
        }
        Logger::Write("Got file contents... parsing cart\n");

        fullCartText = cartStr;

        std::istringstream s(cartStr);
        std::string line;
        std::string currSec = "";
        
        while (std::getline(s, line)) {
            line = utils::trimright(line, " \n\r");
            line = convert_emojis(line);

            if (line.length() > 2 && line[0] == '_' && line[1] == '_') {
                currSec = line;
            }
            else if (currSec == "__lua__"){
                LuaString += line + "\n";
            }
            else if (currSec == "__gfx__"){
                SpriteSheetString += line + "\n";
            }
            else if (currSec == "__gff__"){
                SpriteFlagsString += line + "\n";
            }
            else if (currSec == "__map__"){
                MapString += line + "\n";
            }
            else if (currSec == "__sfx__"){
                SfxString += line + "\n";
            }
            else if (currSec == "__music__"){
                MusicString += line + "\n";
            }
        }

        Logger::Write("Setting cart graphics rom data from strings\n");
        setSpriteSheet(SpriteSheetString);
        setSpriteFlags(SpriteFlagsString);
        setMapData(MapString);

        Logger::Write("Setting cart audio rom data from strings\n");
        setSfx(SfxString);
        setMusic(MusicString);
    }
    else if (hasEnding(filename, ".p8.png")) {
        bool success = loadCartFromPng(filename);

        if (!success){
            return;
        }

        LoadError = "";
        Logger::Write("got valid png cart\n");
    }
    else {
        return;
    }

    const char * patched = getPatchedLua(LuaString.c_str());

    LuaString = patched;
    
    
    #if _TEST
    //run tests to make sure cart is parsed correctly
    //verifyFullCartText(cartStr);

    //verifyCart(this);

    #endif
}

Cart::~Cart(){
    
}

void Cart::initCartRom(){
    //zero out cart rom so no garbage is left over
    for(size_t i = 0; i < sizeof(SpriteSheetData); i++) {
        SpriteSheetData[i] = 0;
    }
    for(size_t i = 0; i < sizeof(SpriteFlagsData); i++) {
        SpriteFlagsData[i] = 0;
    }
    for(size_t i = 0; i < sizeof(MapData); i++) {
        MapData[i] = 0;
    }
    for(size_t i = 0; i < 64; i++) {
        SfxData[i] = {0};
    }
    for(size_t i = 0; i < 64; i++) {
        SongData[i] = {0};
    }
}

void Cart::setSpriteSheet(std::string spritesheetstring){
	Logger::Write("Copying data to spritesheet\n");
	copy_string_to_sprite_memory(SpriteSheetData, spritesheetstring);
}

void Cart::setSpriteFlags(std::string spriteFlagsstring){
	Logger::Write("Copying data to sprite flags\n");
	copy_string_to_memory(SpriteFlagsData, spriteFlagsstring);
}

void Cart::setMapData(std::string mapDataString){
	Logger::Write("Copying data to map data\n");
	copy_string_to_memory(MapData, mapDataString);
}

void Cart::setMusic(std::string musicString){
    std::istringstream s(musicString);
    std::string line;
    char buf[3] = {0};
    int musicIdx = 0;
    
    while (std::getline(s, line)) {
        buf[0] = line[0];
        buf[1] = line[1];
        uint8_t flagByte = (uint8_t)strtol(buf, NULL, 16);

        uint8_t fstop = (flagByte & 4) >> 2;
        uint8_t frepeat = (flagByte & 2) >> 1;
        uint8_t fnext = flagByte & 1;

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

        SongData[musicIdx].data[0] = channel1byte | fnext << 7;
        SongData[musicIdx].data[1] = channel2byte | frepeat << 7;
        SongData[musicIdx].data[2] = channel3byte | fstop << 7;
        SongData[musicIdx].data[3] = channel4byte;

        musicIdx++;
    }

}

void Cart::setSfx(std::string sfxString) {
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

        SfxData[sfxIdx].editorMode = editorMode;
        SfxData[sfxIdx].speed = noteDuration;
        SfxData[sfxIdx].loopRangeStart = loopRangeStart;
        SfxData[sfxIdx].loopRangeEnd = loopRangeEnd;

        //32 notes, 5 chars each
        int noteIdx = 0;
        for (int i = 8; i < 168; i+=5) {
            buf[0] = line[i];
            buf[1] = line[i + 1];
            uint16_t key = (uint16_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 2];
            uint16_t waveform = (uint16_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 3];
            uint16_t volume = (uint16_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 4];
            uint16_t effect = (uint16_t)strtol(buf, NULL, 16);

            SfxData[sfxIdx].notes[noteIdx++] = {
                key,
                waveform,
                volume,
                effect
            };
        }

        sfxIdx++;
    } 
}

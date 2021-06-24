#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <stack>
#include <array>
#include <algorithm>
#include <regex>

#include "lodepng.h"

#include "cart.h"
#include "filehelpers.h"

#include "stringToDataHelpers.h"

#include "utils.h"

#include "logger.h"

#include "emojiconversion.h"

#include "FakoBios.h"


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

    // Find index of a given byte in the structure
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

//implementation from zepto8 code.cpp, pxa_decompress
//https://github.com/samhocevar/zepto8/blob/b1a13516945c49e47495c739e6a43a241ad99291/src/pico8/code.cpp        
static std::string pxa_decompress(uint8_t const *input)
{
    size_t length = input[4] * 256 + input[5];
    size_t compressed = input[6] * 256 + input[7];

    size_t pos = size_t(8) * 8; // stream position in bits
    auto get_bits = [&](size_t count) -> uint32_t
    {
        uint32_t n = 0;
        for (size_t i = 0; i < count && pos < compressed * 8; ++i, ++pos)
            n |= ((input[pos >> 3] >> (pos & 0x7)) & 0x1) << i;
        return n;
    };

    move_to_front mtf;
    std::string ret;

    //TRACE("# Size: %d (%04x)\n", int(compressed), int(compressed));

    while (ret.size() < length && pos < compressed * 8)
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
            //TRACE("%04x [%d] %s\n", int(ret.size()), int(pos-oldpos), printable(ch).c_str());
            ret.push_back(char(ch));
        }
        else
        {
            int nbits = get_bits(1) ? get_bits(1) ? 5 : 10 : 15;
            int offset = get_bits(nbits) + 1;

            if (nbits == 10 && offset == 1)
            {
                uint8_t ch = get_bits(8);
                while (ch)
                {
                    ret.push_back(char(ch));
                    ch = get_bits(8);
                }
                //TRACE("%04x [%d] #%d\n", int(ret.size()), int(pos-oldpos), int(pos-oldpos-21) / 8);
            }
            else
            {
                int n, len = 3;
                do
                    len += (n = get_bits(3));
                while (n == 7);

                //TRACE("%04x [%d] %d@-%d\n", int(ret.size()), int(pos-oldpos), len, offset);
                for (int i = 0; i < len; ++i)
                    ret.push_back(ret[ret.size() - offset]);
                }
        }
    }

    return ret;
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
        Logger_Write("%s%s", LoadError.c_str(), "\n");
        return false;
    }

    if (width != 160 || height != 205) {
        LoadError = "Invalid png dimensions";
        Logger_Write("invalid dimensions\n");
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
            CartRom.SpriteSheetData[picoDataIdx] = extractedByte;
        }
        else if (picoDataIdx < 0x3000) {
            CartRom.MapData[picoDataIdx - 0x2000] = extractedByte;
        }
        else if (picoDataIdx < 0x3100) {
            CartRom.SpriteFlagsData[picoDataIdx - 0x3000] = extractedByte;
        }
        else if (picoDataIdx < 0x3200) {
            size_t offset = picoDataIdx - 0x3100;
            size_t songIdx = offset / sizeof(song);
            size_t channelIdx = offset % sizeof(song);
            CartRom.SongData[songIdx].data[channelIdx] = extractedByte;
        }
        else if (picoDataIdx < 0x4300) {
            size_t offset = picoDataIdx - 0x3200;
            size_t sfxIdx = offset / sizeof(sfx);
            size_t byteIdx = offset % sizeof(sfx);
            CartRom.SfxData[sfxIdx].data[byteIdx] = extractedByte;
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

    Logger_Write("Version: %d Compression %d", version, compression);

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
        LuaString = pxa_decompress(CartLuaData);
    }    

    return true;

}

static std::regex _includeRegex = std::regex("\\s*#include\\s+([\\\\/\\w-\\.]+)");

//tac08 based cart parsing and stripping of emoji
Cart::Cart(std::string filename, std::string cartDirectory){
    //the leading # indicates it is the BBS key. In the future, it would be nice to fetch them,
    //but for now expect the user to supply the carts
    if (filename.length() > 0 && filename[0] == '#') {
        filename = filename.substr(1);
    }

    if (getFileExtension(filename) == "") {
        filename = filename + ".p8";
    }

    if (cartDirectory.length() > 0 && ! isAbsolutePath(filename)) {
        FullCartPath = cartDirectory + "/" + filename;
    }
    else {
        FullCartPath = filename;
    }
    //zero out cart rom so no garbage is left over
    initCartRom();

    Logger_Write("getting file contents\n");
    
    if (hasEnding(FullCartPath, ".p8")){
        std::string cartStr; 

        if (FullCartPath == "__FAKE08-BIOS.p8") {
            cartStr = fake08BiosP8;
        }
        else {
            cartStr = get_file_contents(FullCartPath.c_str());
        }
        Logger_Write("Got file contents... parsing cart\n");

        fullCartText = cartStr;

        std::istringstream s(cartStr);
        std::string line;
        std::string currSec = "";
        std::smatch sm;
        
        while (std::getline(s, line)) {
            line = utils::trimright(line, " \n\r");
            line = charset::utf8_to_pico8(line);
            //line = convert_emojis(line);

            if (line.length() > 2 && line[0] == '_' && line[1] == '_') {
                currSec = line;
            }
            else if (currSec == "__lua__"){
                if (std::regex_match(line, sm, _includeRegex)) {
                    auto dir = getDirectory(FullCartPath);
                    auto fullPath = dir + "/" + sm[1].str();

                    auto includeContents = get_file_contents(fullPath);
                    if (includeContents.length() > 0){
                        includeContents = charset::utf8_to_pico8(includeContents);
                        LuaString += includeContents + "\n";
                    }
                    else{
                        //todo: report error
                        //error: can't find included file
                        return;
                    }
                }
                else {
                    LuaString += line + "\n";
                }
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
            else if (currSec == "__label__"){
                LabelString += line + "\n";
            }
        }

        Logger_Write("Setting cart graphics rom data from strings\n");
        setSpriteSheet(SpriteSheetString);
        setSpriteFlags(SpriteFlagsString);
        setMapData(MapString);

        Logger_Write("Setting cart audio rom data from strings\n");
        setSfx(SfxString);
        setMusic(MusicString);
    }
    else if (hasEnding(FullCartPath, ".png")) {
        bool success = loadCartFromPng(FullCartPath);

        if (!success){
            return;
        }

        LoadError = "";
        Logger_Write("got valid png cart\n");
    }
    else {
        return;
    }
}

Cart::~Cart(){
    
}

void Cart::initCartRom(){
    //zero out cart rom so no garbage is left over
    for(size_t i = 0; i < sizeof(CartRom.SpriteSheetData); i++) {
        CartRom.SpriteSheetData[i] = 0;
    }
    for(size_t i = 0; i < sizeof(CartRom.SpriteFlagsData); i++) {
        CartRom.SpriteFlagsData[i] = 0;
    }
    for(size_t i = 0; i < sizeof(CartRom.MapData); i++) {
        CartRom.MapData[i] = 0;
    }
    for(size_t i = 0; i < 64; i++) {
        CartRom.SfxData[i] = {0};
    }
    for(size_t i = 0; i < 64; i++) {
        CartRom.SongData[i] = {0};
    }
}

void Cart::setSpriteSheet(std::string spritesheetstring){
	Logger_Write("Copying data to spritesheet\n");
	copy_string_to_sprite_memory(CartRom.SpriteSheetData, spritesheetstring);
}

void Cart::setSpriteFlags(std::string spriteFlagsstring){
	Logger_Write("Copying data to sprite flags\n");
	copy_string_to_memory(CartRom.SpriteFlagsData, spriteFlagsstring);
}

void Cart::setMapData(std::string mapDataString){
	Logger_Write("Copying data to map data\n");
	copy_string_to_memory(CartRom.MapData, mapDataString);
}

void Cart::setMusic(std::string musicString){
    std::istringstream s(musicString);
    std::string line;
    char buf[3] = {0};
    int musicIdx = 0;
    
    while (std::getline(s, line)) {
        if (line.length() < 11){
            continue;
        }

        buf[0] = line[0];
        buf[1] = line[1];
        uint8_t flagByte = (uint8_t)strtol(buf, NULL, 16);

        uint8_t mode = (flagByte & 8) >> 3;
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

        CartRom.SongData[musicIdx].data[0] = channel1byte | fnext << 7;
        CartRom.SongData[musicIdx].data[1] = channel2byte | frepeat << 7;
        CartRom.SongData[musicIdx].data[2] = channel3byte | fstop << 7;
        CartRom.SongData[musicIdx].data[3] = channel4byte | mode << 7;

        musicIdx++;
        if (musicIdx >= 64) {
            break;
        }
    }

}

void Cart::setSfx(std::string sfxString) {
    std::istringstream s(sfxString);
    std::string line;
    char buf[3] = {0};
    int sfxIdx = 0;

    //SFX speed defaults to 16. Everything else should be zeroed out
    for (int i = 0; i < 64; i++) {
        CartRom.SfxData[i].speed = 16;
    }
    
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

        CartRom.SfxData[sfxIdx].editorMode = editorMode;
        CartRom.SfxData[sfxIdx].speed = noteDuration;
        CartRom.SfxData[sfxIdx].loopRangeStart = loopRangeStart;
        CartRom.SfxData[sfxIdx].loopRangeEnd = loopRangeEnd;

        //32 notes, 5 chars each
        int noteIdx = 0;
        for (int i = 8; i < 168; i+=5) {
            buf[0] = line[i];
            buf[1] = line[i + 1];
            uint8_t key = (uint8_t)strtol(buf, NULL, 16);
            

            buf[0] = '0';
            buf[1] = line[i + 2];
            uint8_t waveform = (uint8_t)strtol(buf, NULL, 16);
            uint8_t custom = waveform > 7 ? 1 : 0;

            buf[0] = '0';
            buf[1] = line[i + 3];
            uint8_t volume = (uint8_t)strtol(buf, NULL, 16);

            buf[0] = '0';
            buf[1] = line[i + 4];
            uint8_t effect = (uint8_t)strtol(buf, NULL, 16);

            CartRom.SfxData[sfxIdx].notes[noteIdx].setKey(key);
            CartRom.SfxData[sfxIdx].notes[noteIdx].setWaveform(waveform);
            CartRom.SfxData[sfxIdx].notes[noteIdx].setVolume(volume);
            CartRom.SfxData[sfxIdx].notes[noteIdx].setEffect(effect);
            CartRom.SfxData[sfxIdx].notes[noteIdx].setCustom(custom);

            noteIdx++;
        }

        sfxIdx++;
    } 
}

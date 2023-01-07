#include "doctest.h"
#include "../source/fontdata.h"

#include "../source/stringToDataHelpers.h"
#include "../source/nibblehelpers.h"

#define BITMASK(n) (1U<<(n))

TEST_CASE("checking fontdata exists") {
    CHECK(get_font_data().length() == 15995);
}

TEST_CASE("checking defaultFontBinaryData exists") {
    CHECK(defaultFontBinaryData[8*16] > 0);
}

/*
//script to convert sprite sheet formatted font data to binary pico 8 font format
TEST_CASE("convert font data") {
    auto fontdata = get_font_data();
    uint8_t fontSpriteData[128 * 64];
    uint8_t defaultFontData[2048];

    copy_string_to_sprite_memory(fontSpriteData, fontdata);
    

    for(int ch = 0; ch < 256; ch++) {
        //first 16 chars don't print
        //TODO: fill in default width data
        if (ch < 16) {
            for(int j = 0; j < 8; j++) {
                defaultFontData[(ch*8) + j] = 0;
            }

            continue;
        }

        if (ch >= 0x10) {
			int index = ch - 0x10;
			int width = 4;
            int height = 5;
            int sprX = (index % 16) * 8;
            int sprY = (index / 16) * 8;

            if (ch >= 0x80) {
                index = ch - 0x80;
                width = 8;
                sprY =  (index / 16) * 8 + 56;
            }

            //copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8, width, height, false, false);
			//copySpriteToScreen(fontSpriteData, x, y, (index % 16) * 8, (index / 16) * 8 + 56, width, height, false, false);

            //each char is 8x8 bitfield. default chars should only take up 4x5 (or 8x5 for wide chars)
            //get the sprite index, then get the pixel nibble, and if its set set that bit to 1
            //combine all 8 bits into a byte, and add it to the buffer, then do the same for the next line
            for(int y = 0; y < 8; y++) {
                uint8_t lineBitfield = 0;
                if (y < height) {
                    for (int x = 0; x < width; x+=2) {
                        uint8_t bothPix = fontSpriteData[COMBINED_IDX(sprX + x, sprY + y)];

                        uint8_t lc = bothPix & 0x0f;
                        uint8_t rc = bothPix >> 4;

                        if (lc) {
                            lineBitfield |= BITMASK(x);
                        }
                        if (rc) {
                            lineBitfield |= BITMASK(x + 1);
                        }
                    }
                }

                defaultFontData[(ch*8) + y] = lineBitfield;
            }
        }
			
		
        printf("finished converting font to binary data. printing\n\n");
        printf("uint8_t defaultFontBinaryData[2048] = {\n");

        for (int i = 0; i < 2048; i++) {
            printf("%d, ", defaultFontData[i]);

            if ((i + 1) % 8 == 0) {
                printf("// char %d\n", i / 8);
            }
        }

        printf("}\n\n");
        printf("done printing\n\n");
    }
}
*/
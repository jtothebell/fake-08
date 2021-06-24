
//
//  ZEPTO-8 — Fantasy console emulator
//
//  Copyright © 2016—2020 Sam Hocevar <sam@hocevar.net>
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//


// I know codecvt_utf8 is deprecated, but let’s hope C++ comes with a
// replacement before they actually remove the feature.
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING 1


#include <locale>
#include <string>
#include <codecvt>
#include <cstring>
#include <regex>
#include <string_view>

#include "emojiconversion.h"


std::string_view charset::to_utf8[256];
std::u32string_view charset::to_utf32[256];

static uint8_t multibyte_start[256];
static std::map<std::string, uint8_t> to_pico8;
std::regex charset::utf8_regex = charset::static_init();

std::regex charset::static_init()
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;

    // The complete PICO-8 charmap, from 0 to 255. We cannot just store
    // codepoints because some emoji glyphs are combinations of several
    // codepoints, e.g. ⬇️ is U+2B07 (down arrow) + U+FE0F (variation
    // selector-16).
    static char const utf8_chars[] =
        "\0¹²³⁴⁵⁶⁷⁸\t\nᵇᶜ\rᵉᶠ▮■□⁙⁘‖◀▶「」¥•、。゛゜"
        " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNO"
        "PQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~○"
        "█▒🐱⬇️░✽●♥☉웃⌂⬅️😐♪🅾️◆…➡️★⧗⬆️ˇ∧❎▤▥あいうえおか"
        "きくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよ"
        "らりるれろわをんっゃゅょアイウエオカキクケコサシスセソタチツテト"
        "ナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンッャュョ◜◝";
    static auto utf32_chars = cvt.from_bytes(utf8_chars, &utf8_chars[sizeof(utf8_chars)]);

    // Create all sorts of lookup tables for PICO-8 character conversions
    char const *p8 = utf8_chars;
    auto const *p32 = (char32_t const *)utf32_chars.data();
    std::string regex("(");
    for (int i = 0; i < 256; ++i)
    {
        size_t len32 = p32[1] == 0xfe0f ? 2 : 1;
        size_t len8 = ((0xe5000000 >> ((*p8 >> 3) & 0x1e)) & 3) + len32 * len32;
        to_utf8[i] = std::string_view(p8, len8);
        to_utf32[i] = std::u32string_view(p32, len32);
        to_pico8[std::string(p8, len8)] = i;

        // Build a regex that lets us do faster (maybe?) UTF-8 conversions
        if (len8 > 1)
        {
            multibyte_start[(uint8_t)*p8] = 1;
            regex += std::string(p8, len8) + '|';
        }

        p8 += len8;
        p32 += len32;
    }
    regex += ')'; // Fall back to an empty match on purpose

    return std::regex(regex);
}

std::string charset::utf8_to_pico8(std::string const &str)
{
    std::string ret;
    std::smatch sm;

    for (auto p = str.begin(); p != str.end(); )
    {
        // Only pass known start characters through the expensive regex
        if (multibyte_start[(uint8_t)*p]
             && std::regex_search(p, str.end(), sm, utf8_regex)
             && sm.length() > 1)
        {
            ret += to_pico8[sm.str()];
            p += sm.length();
        }
        else
        {
            ret += *p++;
        }
    }

    return ret;
}

std::string charset::pico8_to_utf8(std::string const &str)
{
    std::string ret;
    for (uint8_t ch : str)
        ret += std::string(to_utf8[ch]);
    return ret;
}


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

#pragma once

#include <cstdint>

namespace z8
{

// Synth parameters for waveform generation
struct synth_param
{
    uint8_t instrument = 0;
    bool custom = false;
    uint8_t filters = 0;
    uint8_t key = 0;
    float freq = 0;
    float volume = 0;
    float phi = 0;
    float last_advance = 0;
    float last_sample = 0;
    bool is_music = false;
};

//
// A waveform generator
//

class synth
{
public:
    enum
    {
        INST_TRIANGLE   = 0, // Triangle signal
        INST_TILTED_SAW = 1, // Slanted triangle
        INST_SAW        = 2, // Sawtooth
        INST_SQUARE     = 3, // Square signal
        INST_PULSE      = 4, // Asymmetric square signal
        INST_ORGAN      = 5, // Some triangle stuff again
        INST_NOISE      = 6,
        INST_PHASER     = 7,
    };

    static float waveform(synth_param &params);
};

} // namespace z8

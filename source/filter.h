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

namespace z8
{

//
// Biquad filter
//

class filter
{
public:
    enum class type
    {
        lpf, hpf, lowshelf, highshelf
    };

    filter() = default;
    filter(type t, float freq, float q, float gain);

    void init(type t, float freq, float q, float gain);
    float run(float input);

    float c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0;
    float linput = 0;
    float llinput = 0;
    float loutput = 0;
    float lloutput = 0;
};

} // namespace z8


//
//  ZEPTO-8 — Fantasy console emulator
//
//  Copyright © 2017–2024 Sam Hocevar <sam@hocevar.net>
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#include "filter.h"

#include <cmath> // std::cos, std::sin, std::pow, std::sqrt

namespace z8
{

static constexpr float F_TAU = 6.283185307179586f;

filter::filter(type t, float freq, float q, float gain)
{
    init(t, freq, q, gain);
}

void filter::init(type t, float freq, float q, float gain)
{
    // Formulae are from https://www.w3.org/TR/audio-eq-cookbook/
    float w0 = F_TAU * freq / 22050;
    float cosw0 = std::cos(w0);
    float a = std::pow(10.0f, gain / 40);
    float alpha;
    if (t == type::lowshelf || t == type::highshelf)
        alpha = (std::sin(w0) / 2) * std::sqrt((a + 1 / a) * (1 / q - 1) + 2);
    else
        alpha = std::sin(w0) / (2 * q);

    float a0, a1, a2, b0, b1, b2;
    switch (t)
    {
    case type::lpf:
        a0 =  1 + alpha;
        a1 = -2 * cosw0;
        a2 =  1 - alpha;
        b0 = (1 - cosw0) / 2;
        b1 = (1 - cosw0);
        b2 = (1 - cosw0) / 2;
        break;
    case type::hpf:
        a0 =  1 + alpha;
        a1 = -2 * cosw0;
        a2 =  1 - alpha;
        b0 =  (1 + cosw0) / 2;
        b1 = -(1 + cosw0);
        b2 =  (1 + cosw0) / 2;
        break;
    case type::lowshelf:
    {
        float sqra = 2 * std::sqrt(a) * alpha;
        a0 = (a + 1) + (a - 1) * cosw0 + sqra;
        a1 = -2 * ((a - 1) + (a + 1) * cosw0);
        a2 = (a + 1) + (a - 1) * cosw0 - sqra;
        b0 = a * ((a + 1) - (a - 1) * cosw0 + sqra);
        b1 = 2 * a * ((a - 1) - (a + 1) * cosw0);
        b2 = a * ((a + 1) - (a - 1) * cosw0 - sqra);
        break;
    }
    case type::highshelf:
    {
        float sqra = 2 * std::sqrt(a) * alpha;
        a0 = (a + 1) - (a - 1) * cosw0 + sqra;
        a1 = 2 * ((a - 1) - (a + 1) * cosw0);
        a2 = (a + 1) - (a - 1) * cosw0 - sqra;
        b0 = a * ((a + 1) + (a - 1) * cosw0 + sqra);
        b1 = -2 * a * ((a - 1) + (a + 1) * cosw0);
        b2 = a * ((a + 1) + (a - 1) * cosw0 - sqra);
        break;
    }
    }
    c1 = b0 / a0;
    c2 = b1 / a0;
    c3 = b2 / a0;
    c4 = a1 / a0;
    c5 = a2 / a0;
}

float filter::run(float input)
{
    float output = c1 * input + c2 * linput + c3 * llinput - c4 * loutput - c5 * lloutput;
    llinput = linput;
    linput = input;
    lloutput = loutput;
    loutput = output;

    return output;
}

} // namespace z8


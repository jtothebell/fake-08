#pragma once

//std::clamp is in c++17, but not c++11
int clamp (int val, int lo, int hi);

float clamp (float val, float lo, float hi);

float lerp (float a, float b, float t);


#include "mathhelpers.h"

//std::clamp is in c++17, but not c++11
int clamp (int val, int lo, int hi) {
	val = (val > hi) ? hi : val;
    return (val < lo) ? lo : val;
}

float clamp (float val, float lo, float hi) {
	val = (val > hi) ? hi : val;
    return (val < lo) ? lo : val;
}

float lerp(float a, float b, float t) {
	return (b - a) * t + a;
}

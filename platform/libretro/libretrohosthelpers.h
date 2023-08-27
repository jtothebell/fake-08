#include <string.h>
#include <cstdint>


void setInputState(
    uint8_t kDown,
    uint8_t kHeld,
    int16_t mouseX,
    int16_t mouseY,
    uint8_t mouseBtnState);

void setCartDirectory(std::string dir);
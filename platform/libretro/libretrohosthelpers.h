#include <string.h>

#if (defined(_WIN32) || defined(__WIN32__))
#include <direct.h> /* _mkdir */
#define mkdir(A, B) _mkdir(A)
#endif

void setInputState(
    uint8_t kDown,
    uint8_t kHeld,
    int16_t mouseX,
    int16_t mouseY,
    uint8_t mouseBtnState);

void setCartDirectory(std::string dir);
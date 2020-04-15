#include "test_base.h"

#if _TEST

#pragma once

#include <string>

#include "../cart.h"

bool verifyFullCartText(std::string cartText);

bool verifyCart(Cart* cart);
bool verifyLuaText(std::string luaText);
bool verifySpriteSheetText(std::string spritesheet);

#endif

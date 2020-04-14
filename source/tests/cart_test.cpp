#include <stdio.h>
#include <3ds.h>

#include <string>

#include "cart_test.h"

bool verifyFullCartText(std::string cartText){
    printf("test printing from cart_test.cpp\n");
    printf(cartText.c_str());
    return true;
}

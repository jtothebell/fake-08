#include <string>

#include "cart.h"
#include "filehelpers.h"

#include "tests/test_switch.h"
#if _TEST
#include "tests/cart_test.h"
#endif

void LoadCart(std::string filename){
    auto cartStr = get_file_contents(filename.c_str());

    /*
    PicoCart cart = {
        filename,
        cartStr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };
    */

    
    #if _TEST
    verifyFullCartText(cartStr);

    #endif

    //return cart;
}
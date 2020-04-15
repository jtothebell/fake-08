#include <string>

#include "console.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"

Console::Console(){
    auto fontdata = get_font_data();

    initPico8Graphics(fontdata);
}

void Console::LoadCart(std::string filename){
    Cart cart = Cart(filename);

    _loadedCart = &cart;
    
}

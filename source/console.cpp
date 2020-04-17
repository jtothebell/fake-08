#include <string>

#include "console.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"
#include "picoglobalapi.h"

Console::Console(){
    auto fontdata = get_font_data();

    Graphics* graphics = new Graphics(fontdata);

    _graphics = graphics;

    //this can probably go away when I'm loading actual carts and just have to expose api to lua
    initGlobalApi(_graphics);
}

void Console::LoadCart(std::string filename){
    Cart cart = Cart(filename);

    _loadedCart = &cart;
    
}

void Console::FlipBuffer(uint8_t* fb){
    _graphics->flipBuffer(fb);
}

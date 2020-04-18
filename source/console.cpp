#include <string>

#include "console.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"
#include "picoglobalapi.h"
#include "logger.h"

Console::Console(){
    Logger::Write("getting font string\n");
    auto fontdata = get_font_data();

    Logger::Write("Creating Graphics object\n");
    Graphics* graphics = new Graphics(fontdata);
    Logger::Write("Created Graphics object\n");

    _graphics = graphics;

    //this can probably go away when I'm loading actual carts and just have to expose api to lua
    Logger::Write("Initializing global api\n");
    initGlobalApi(_graphics);
}

void Console::LoadCart(std::string filename){
    Logger::Write("Calling Cart Constructor\n");
    Cart cart = Cart(filename);

    _loadedCart = &cart;

    _graphics->setSpriteSheet(_loadedCart->SpriteSheetString);
}

void Console::FlipBuffer(uint8_t* fb){
    _graphics->flipBuffer(fb);
}

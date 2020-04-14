#include <string>

#include "picointernals.h"
#include "graphics.h"
#include "fontdata.h"
#include "cart.h"

void initPicoInternals() {
    auto fontdata = get_font_data();

    initPico8Graphics(fontdata);
    
}

void loadcart(std::string filename){
    //auto cart = LoadCart(filename);
    LoadCart(filename);
    
}
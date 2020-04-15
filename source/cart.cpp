#include <string>
#include <sstream>

#include "cart.h"
#include "filehelpers.h"

#include "utils.h"

#include "tests/test_base.h"
#if _TEST
#include "tests/cart_test.h"
#endif

void LoadCart(std::string filename){
    auto cartStr = get_file_contents(filename.c_str());

    std::istringstream s(cartStr);
    std::string line;
    std::string lua = "";
    std::string spritesheet = "";
    std::string currSec = "";
    
    while (std::getline(s, line)) {
		line = utils::trimright(line, " \n\r");

        if (line.length() > 2 && line[0] == '_' && line[1] == '_') {
            currSec = line;
        }
        else if (currSec == "__lua__"){
            lua += line + "\n";
        }
        else if (currSec == "__gfx__"){
            spritesheet += line + "\n";
        }
	}

    
    #if _TEST
    //run tests to make sure cart is parsed correctly
    verifyFullCartText(cartStr);

    verifyLuaText(lua);

    verifySpriteSheetText(spritesheet);

    #endif

    //return cart;
}
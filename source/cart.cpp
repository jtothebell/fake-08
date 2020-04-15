#include <string>
#include <sstream>

#include "cart.h"
#include "filehelpers.h"

#include "utils.h"

#include "tests/test_base.h"
#if _TEST
#include "tests/cart_test.h"
#endif

Cart::Cart(std::string filename){
    Filename = filename;
    auto cartStr = get_file_contents(filename.c_str());

    fullCartText = cartStr;

    std::istringstream s(cartStr);
    std::string line;
    std::string currSec = "";
    
    while (std::getline(s, line)) {
		line = utils::trimright(line, " \n\r");

        if (line.length() > 2 && line[0] == '_' && line[1] == '_') {
            currSec = line;
        }
        else if (currSec == "__lua__"){
            LuaString += line + "\n";
        }
        else if (currSec == "__gfx__"){
            SpriteSheetString += line + "\n";
        }
        else if (currSec == "__gff__"){
            SpriteFlagsString += line + "\n";
        }
        else if (currSec == "__map__"){
            MapString += line + "\n";
        }
        else if (currSec == "__sfx__"){
             //no sound support yet
        }
        else if (currSec == "__sfx__"){
             //no sound support yet
        }
	}

    
    #if _TEST
    //run tests to make sure cart is parsed correctly
    verifyFullCartText(cartStr);

    verifyCart(this);

    #endif
}

#include <string>
#include <sstream>

#include "cart.h"
#include "filehelpers.h"

#include "utils.h"

#include "tests/test_switch.h"
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
		
        /*
		if (!check_include_file(line, cart, filenum)) {
			if (valid_sections.find(line) != valid_sections.end()) {
				cart.sections["cur_sect"] = line;
				logr << "section " << line;
			} else {
				if (cart.sections["cur_sect"] == "__lua__") {
					cart.source.push_back(Line{filenum, line});
				} else {
					cart.sections[cart.sections["cur_sect"]] += line + "\n";
				}
			}
		}
        */
	}

    
    #if _TEST
    //run tests to make sure cart is parsed correctly
    verifyFullCartText(cartStr);

    verifyLuaText(lua);

    verifySpriteSheetText(spritesheet);

    #endif

    //return cart;
}
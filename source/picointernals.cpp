#include <string>

#include "picointernals.h"
#include "graphics.h"
#include "fontdata.h"

void initPicoInternals() {
    auto fontdata = get_font_data();

    initPico8Graphics(fontdata);
    
}
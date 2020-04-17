
#include <string>

#include "graphics.h"

Graphics* _graphicsForApi;

void initGlobalApi(Graphics* graphics){
    _graphicsForApi = graphics;
}

void cls(){
    _graphicsForApi->cls();
}
void pset(short x, short y, uint8_t col){
    _graphicsForApi->pset(x, y, col);
}
uint8_t pget(short x, short y){
    return _graphicsForApi->pget(x, y);
}
void color(uint8_t c){
    _graphicsForApi->color(c);
}
void line (short x1, short y1, short x2, short y2, uint8_t col){
    _graphicsForApi->line(x1, y1, x2, y2, col);
}
void circ(short ox, short oy, short r, uint8_t col){
    _graphicsForApi->circ(ox, oy, r, col);
}
void circfill(short ox, short oy, short r, uint8_t col){
    _graphicsForApi->circfill(ox, oy, r, col);
}
void rect(short x1, short y1, short x2, short y2, uint8_t col){
    _graphicsForApi->rect(x1, y1, x2, y2, col);
}
void rectfill(short x1, short y1, short x2, short y2, uint8_t col){
    _graphicsForApi->rectfill(x1, y1, x2, y2, col);
}
short print(std::string str, short x, short y, uint16_t c){
    return _graphicsForApi->print(str, x, y, c);
}

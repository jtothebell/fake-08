
#include <string>

#include "picoglobalapi.h"
#include "graphics.h"


Graphics* _graphicsForGlobalCApi;

void initGlobalApi(Graphics* graphics){
    _graphicsForGlobalCApi = graphics;
}

void cls(){
    _graphicsForGlobalCApi->cls();
}
void pset(short x, short y, uint8_t col){
    _graphicsForGlobalCApi->pset(x, y, col);
}
uint8_t pget(short x, short y){
    return _graphicsForGlobalCApi->pget(x, y);
}
void color(uint8_t c){
    _graphicsForGlobalCApi->color(c);
}
void line (short x1, short y1, short x2, short y2, uint8_t col){
    _graphicsForGlobalCApi->line(x1, y1, x2, y2, col);
}
void circ(short ox, short oy, short r, uint8_t col){
    _graphicsForGlobalCApi->circ(ox, oy, r, col);
}
void circfill(short ox, short oy, short r, uint8_t col){
    _graphicsForGlobalCApi->circfill(ox, oy, r, col);
}
void rect(short x1, short y1, short x2, short y2, uint8_t col){
    _graphicsForGlobalCApi->rect(x1, y1, x2, y2, col);
}
void rectfill(short x1, short y1, short x2, short y2, uint8_t col){
    _graphicsForGlobalCApi->rectfill(x1, y1, x2, y2, col);
}
short print(std::string str, short x, short y, uint16_t c){
    return _graphicsForGlobalCApi->print(str, x, y, c);
}
void spr(int n, int x, int y, int w, int h, bool flip_x, bool flip_y) {
    _graphicsForGlobalCApi->spr(n, x, y, w, h, flip_x, flip_y);
}

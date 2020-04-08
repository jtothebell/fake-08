#include <3ds.h>

#include "graphics.h"


float r1x0 = 10;
float r1y0 = 10;
float r1x1 = 50;
float r1y1 = 50;
float r1c = 2;

float r2x0 = 60;
float r2y0 = 60;
float r2x1 = 100;
float r2y1 = 100;
float r2c = 3;

float r1dx = .5;
float r1dy = 2;

float r2dx = 2;
float r2dy = 2;

void _draw() {
    
    cls();

    rect(10, 10, 50, 50, 2);

    rectfill((int)r1x0, (int)r1y0, (int)r1x1, (int)r1y1, r1c);

    rectfill((int)r2x0, (int)r2y0, (int)r2x1, (int)r2y1, r2c);
}

void _update() {
    if (r1x0 < 0 || r1x1 > 128){
        r1dx = -1 * r1dx;
    }

    if (r1y0 < 0 || r1y1 > 128){
        r1dy = -1 * r1dy;
    }

    if (r2x0 < 0 || r2x1 > 128){
        r2dx = -1 * r2dx;
    }

    if (r2y0 < 0 || r2y1 > 128){
        r2dy = -1 * r2dy;
    }

    r1x0 = r1dx + r1x0;
    r1x1 = r1dx + r1x1;
    r1y0 = r1dy + r1y0;
    r1y1 = r1dy + r1y1;
    
}

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

    rectfill(0, 0, 127, 127, 0);

    print("abcdefghijklmnopqrstuvwxyz", 0, 0, 7);

    rectfill(50, -1, 60, 10, 11);

    rectfill((short)r1x0, (short)r1y0, (short)r1x1, (short)r1y1, r1c);

    rect((short)r2x1, (short)r2y1, (short)r2x0, (short)r2y0, r2c);

    pset(40, 10, 13);

    line(120, 10, 120, 120, 14);

    line(0, 0, 20, 20, 15);

    line(0, 20, 20, 0, 15);

    line(20, 30, 0, 20, 12);

    line(0, 30, 20, 20, 12);

    circ(90, 90, 30, 8);

    circfill(100, 100, 15, 9);

    print("test str", 64, 64, 7);
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
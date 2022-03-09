#include <vector>
#include "doctest.h"

#include "testHelpers.h"


bool colorsEqual(Color* lhs, Color* rhs) {
	return lhs->Alpha == rhs->Alpha &&
		   lhs->Red == rhs->Red &&
		   lhs->Green == rhs->Green &&
		   lhs->Blue == rhs->Blue;
}

void debugScreen(Graphics* graphics) {
    for (int x=0; x < 128; x++) {
        for (int y = 0; y < 128; y++) {
 	        uint8_t c = graphics->pget(x,y);
            if (c != 0) {
                printf("%d,%d,%d\n", x, y, c);
            }
        }
    }
}

void checkPoints(Graphics* graphics, std::vector<coloredPoint> expectedPoints) {
    bool isCorrect = true;
    for(size_t i = 0; i < expectedPoints.size(); i++){
        auto toCheck = expectedPoints[i];
        isCorrect &= graphics->pget(toCheck.x, toCheck.y) == toCheck.c;
    }

    CHECK(isCorrect);
}

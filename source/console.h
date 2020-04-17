#pragma once

#include "cart.h"

class Console {
    Cart* _loadedCart;
    Graphics* _graphics;

    public:
    Console();

    void LoadCart(std::string filename);

    void FlipBuffer(uint8_t* fb);
};


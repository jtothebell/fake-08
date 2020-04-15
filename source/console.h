#pragma once

#include "cart.h"

class Console {
    Cart* _loadedCart;

    public:
    Console();

    void LoadCart(std::string filename);
};


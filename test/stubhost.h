#pragma once

#include "../source/host.h"

class StubHost : public Host { 
    public:
    StubHost();      
    void stubInput(uint8_t kdown, uint8_t kheld);
};
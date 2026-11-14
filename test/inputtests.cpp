#include <vector>

#include "doctest.h"

#include "../source/Input.h"
#include "../source/PicoRam.h"
#include "../source/hostVmShared.h"

// Holds button 0 for `calls` calls to SetState (a press on the first) and
// returns the calls, counted from 1, on which btnp(0) is true.
static std::vector<int> btnpCalls(Input* input, int calls) {
    std::vector<int> result;
    for (int c = 1; c <= calls; c++) {
        input->SetState(c == 1 ? BITMASK(0) : 0, BITMASK(0));
        if (input->btnp(0)) {
            result.push_back(c);
        }
    }
    input->SetState(0, 0);
    return result;
}

TEST_CASE("btnp repeat") {
    PicoRam* memory = new PicoRam();
    memory->Reset();
    Input* input = new Input(memory);

    SUBCASE("at 30 fps: pressed, again after 15 frames, then every 4") {
        CHECK_EQ(btnpCalls(input, 30), std::vector<int>{1, 15, 19, 23, 27});
    }
    SUBCASE("at 60 Hz the same times take twice the calls") {
        input->SetFrameScale(2);
        CHECK_EQ(btnpCalls(input, 60), std::vector<int>{1, 30, 38, 46, 54});
    }
    SUBCASE("custom delay (0x5f5c) and interval (0x5f5d)") {
        memory->hwState.btnpRepeatDelay = 5;
        memory->hwState.btnpRepeatInterval = 3;
        CHECK_EQ(btnpCalls(input, 15), std::vector<int>{1, 5, 8, 11, 14});
    }
    SUBCASE("a delay of 255 turns repeat off") {
        memory->hwState.btnpRepeatDelay = 255;
        CHECK_EQ(btnpCalls(input, 40), std::vector<int>{1});
    }
    SUBCASE("releasing the button restarts the count") {
        btnpCalls(input, 10);
        CHECK_EQ(btnpCalls(input, 16), std::vector<int>{1, 15});
    }
    SUBCASE("btn follows the held buttons") {
        input->SetState(BITMASK(2), BITMASK(2));
        CHECK(input->btn(2));
        input->SetState(0, BITMASK(2));
        CHECK(input->btn(2));
        CHECK_FALSE(input->btnp(2));
        CHECK_EQ(memory->hwState.buttonStates[0], BITMASK(2));
    }
    SUBCASE("the pause button counts for one frame only") {
        input->SetState(BITMASK(6), BITMASK(6));
        CHECK(input->btn(6));
        input->SetState(0, BITMASK(6));
        CHECK_FALSE(input->btn(6));
    }

    delete input;
    delete memory;
}

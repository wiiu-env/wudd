#pragma once
#include "Input.h"
class CombinedInput : public Input {
public:
    void combine(const Input &b) {
        data.buttons_h |= b.data.buttons_h;
    }

    void process() {
        data.buttons_d |= (data.buttons_h & (~lastData.buttons_h));
        data.buttons_r |= (lastData.buttons_h & (~data.buttons_h));
        lastData.buttons_h = data.buttons_h;
    }

    void reset() {
        data.buttons_h = 0;
        data.buttons_d = 0;
        data.buttons_r = 0;
    }
};
#pragma once

namespace mana {

class DCBlocker {
public:
    void Init(float sampleRate) {}
    float Process(float in) {
        auto t = in - latch_;
        latch_ = in;
        auto y = t + 0.9995f * latch1_;
        latch1_ = y;
        return y;
    }
    void ClearInternal() {
        latch_ = 0;
        latch1_ = 0;
    }
private:
    float latch_ = 0;
    float latch1_ = 0;
};

}
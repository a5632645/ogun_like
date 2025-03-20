#pragma once
#include <vector>

namespace mana {

class Delay {
public:
    void  Init(float maxDelay);
    void  SetDelay(float delay);
    float GetLast() const;
    void  Push(float x);
    float Process(float x);
    void  Panic();
private:
    std::vector<float> buffer_;
    int wpos_ = 0;
    float delay_ = 0;
    int maxDelay_ = 0;
};

}

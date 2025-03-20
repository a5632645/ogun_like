#include "Delay.hpp"
#include <cmath>

namespace mana {

static constexpr int Nearst2Power(int x) {
    int ret = 1;
    while (ret < x) {
        ret *= 2;
    }
    return ret;
}

void Delay::Init(float maxDelay) {
    auto iMaxDelay = static_cast<int>(std::ceil(maxDelay));
    auto len = Nearst2Power(iMaxDelay);
    buffer_.resize(len);
    maxDelay_ = len;
}

void Delay::SetDelay(float delay) {
    if (delay < 0) delay = 0;
    delay_ = delay;
}

float Delay::GetLast() const {
    const auto mask = buffer_.size() - 1;
    auto rpos = static_cast<int>(wpos_ - delay_ - 1);
    rpos &= mask;
    return buffer_[rpos];
}

void Delay::Push(float x) {
    const auto mask = buffer_.size() - 1;
    buffer_[wpos_++] = x;
    wpos_ &= mask;
}

float Delay::Process(float x) {
    Push(x);
    return GetLast();
}

void Delay::Panic() {
    std::fill(buffer_.begin(), buffer_.end(), 0);
}

} // namespace mana

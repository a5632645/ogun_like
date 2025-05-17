#include "quad_osc.hpp"
#include <cmath>

namespace ogun {

void QuadOSC::SetFreq(float omega) {
    k1_ = std::tan(omega / 2.0f);
    k2_ = 2.0f * k1_ / (1.0f + k1_ * k1_);
}

void QuadOSC::Tick() {
    float w = u_ - k1_ * v_;
    v_ = v_ + k2_ * w;
    u_ = w - k1_ * v_;
}

}

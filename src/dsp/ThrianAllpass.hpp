#pragma once
#include <algorithm>
#include <array>
#include <complex>

namespace mana {

class ThrianAllpass {
public:
    static constexpr uint32_t kMaxNumAPF = 8;

    void  Init(float sampleRate);
    float Process(float in);
    float GetPhaseDelay(float freq) const;
    void  SetGroupDelay(float delay);
    void  Panic();
    std::complex<float> GetResponce(float omega) const;
private:
    float ProcessFilter(float in, uint32_t i);

    float sampleRate_{};
    float a2_{};
    float a1_{};
    float b0_{};
    float b2_{};
    struct APFData {
        float latch1_{};
        float latch2_{};
    } latchs_[kMaxNumAPF]{};
};

}

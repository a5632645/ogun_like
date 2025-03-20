#pragma once

namespace mana {

class Lowpass {
public:
    enum class LoopFilterType {
        IIR_LPF1,
        IIR_LPF2,
        FIR_LPF
    };

    void  Init(float sampleRate);
    float Process(float x);
    void  SetCutOffFreq(float freq);
    void  SetLPF1(float freq);
    void  SetLPF2(float freq);
    void  SetLPF1FIR(float freq);
    float GetPhaseDelay(float freq) const;
    float GetGroupDelay(float freq) const;
    float GetGainPower2(float freq) const;
    void  Panic();
    float GetMaxFreq() const;
    void  SetLoopFilterType(LoopFilterType type);
    LoopFilterType GetLoopFilterType() const { return loopFilterType_; }
private:
    LoopFilterType loopFilterType_{ LoopFilterType::IIR_LPF2 };
    float freq_{};
    float sampleRate_{};
    float latch1_{};
    float latch2_{};
    float a1_{};
    float a2_{};
    float b0_{};
    float b1_{};
    float b2_{};
};

}

#pragma once
#include "Delay.hpp"
#include "Lowpass.hpp"
#include "ThrianAllpass.hpp"
#include "MixMatrix.hpp"
#include "DCBlocker.hpp"

namespace mana {

class Resonator {
public:
    static constexpr int kNumPath = MixMatrix::kSize;
    static constexpr int kMaxPitch = 127;
    static constexpr int kMinPitch = 0;

    Resonator();

    void  Init(float sampleRate);
    float Process(float x);
    void  Panic();

    void  SetPitchShift(float pitch, int idx);
    void  SetDetune(float detune, int idx);
    void  SetDecay(float decay, int idx);
    void  SetDecaySign(bool decay, int idx);
    void  SetDispersion(float dispersion, int idx);
    void  SetDampPitch(float pitch, int idx);
    void  SetOutputMix(float mix, int idx);
    void  SetGlobalPitch(float pitch);

    void  MakeUnit() { mixMatrix_.MakeUnit(); }
    void  MakeHammond() { mixMatrix_.MakeHammond(); }
    void  MakeHammondSplit() { mixMatrix_.MakeHammondSplit(); }
private:
    void CalcDelayLen(int idx);
    void CalcDecay(int idx);
    
    float sampleRate_{};
    float globalPitch_{};
    struct Path {
        bool disable_{};
        float pitchShift_{};
        float detune_{};
        float freq_{};
        float dampPitch_{};
        float dispersionRatio_{};
        float delayLen_{};
        float decay_{};
        bool decaySign_{};
        float realDecay_{};
        Delay delay_;
        Lowpass lowpass_;
        ThrianAllpass thrianAllpass_;
        DCBlocker dcBlocker_;
    };
    Path paths_[kNumPath];
    float outputMix_[kNumPath]{};
    MixMatrix mixMatrix_;
};

}
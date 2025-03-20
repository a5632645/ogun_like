#include "Resonator.hpp"
#include <cmath>
#include <cassert>

static constexpr int kMinPitch = 0;

static float PitchToFreq(float pitch) {
    auto pdiff = pitch - 69;
    auto freq = 440.0f;
    return freq * std::exp2(pdiff / 12.0f);
}

static float PitchToTime(float pitch) {
    return 1.0f / PitchToFreq(pitch);
}

static float PitchToSamples(float pitch, float sampleRate) {
    return PitchToTime(pitch) * sampleRate;
}

static float ClampUncheck(float x, float min, float max) {
    return x < min ? min : x > max ? max : x;
}

namespace mana {

Resonator::Resonator() {
    std::fill_n(outputMix_, kNumPath, 0.0f);
}

void Resonator::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    for (auto& p : paths_) {
        p.delay_.Init(PitchToSamples(kMinPitch, sampleRate));
        p.lowpass_.Init(sampleRate);
        p.thrianAllpass_.Init(sampleRate);
    }
}

float Resonator::Process(float x) {
    std::array<float, kNumPath> temp{};

    for (int i = 0; i < kNumPath; ++i) {
        auto& p = paths_[i];
        auto ret = p.realDecay_ * p.delay_.GetLast();
        temp[i] = ret + x;
    }

    auto mix = mixMatrix_.Process(temp);

    for (int i = 0; i < kNumPath; ++i) {
        auto& p = paths_[i];
        if (p.disable_) {
            p.delay_.Push(mix[i]);
        }
        else {
            auto fb = p.thrianAllpass_.Process(mix[i]);
            fb = p.dcBlocker_.Process(fb);
            assert(!std::isnan(fb));
            fb = p.lowpass_.Process(fb);
            fb = ClampUncheck(fb, -16.0f, 16.0f);
            p.delay_.Push(fb);
        }
    }

    float out = 0.0f;
    for (int i = 0; i < kNumPath; ++i) {
        out += mix[i] * outputMix_[i];
    }
    return out;
}

void Resonator::Panic() {
    for (auto& p : paths_) {
        p.delay_.Panic();
        p.lowpass_.Panic();
        p.thrianAllpass_.Panic();
    }
}

void Resonator::SetPitchShift(float pitch, int idx) {
    auto& p = paths_[idx];
    p.pitchShift_ = pitch;
    CalcDelayLen(idx);
}

void Resonator::SetDetune(float detune, int idx) {
    auto& p = paths_[idx];
    p.detune_ = detune;
    CalcDelayLen(idx);
}

static float DbToGain(float db) {
    return std::pow(10.0f, db / 20.0f);
}
void Resonator::SetDecay(float decay, int idx) {
    auto& p = paths_[idx];
    p.decay_ = decay;
    CalcDecay(idx);
}

void Resonator::CalcDecay(int idx) {
    auto& p = paths_[idx];
    auto v = std::pow(10.0f, -p.delayLen_ / (sampleRate_ * p.decay_ / 1000.0f));
    if (p.decaySign_) { // positive
        p.realDecay_ = v;
    }
    else {
        p.realDecay_ = -v;
    }
}

void Resonator::SetDecaySign(bool decay, int idx) {
    auto& p = paths_[idx];
    p.decaySign_ = decay;
    CalcDelayLen(idx);
}

void Resonator::SetDispersion(float dispersion, int idx) {
    auto& p = paths_[idx];
    p.dispersionRatio_ = dispersion;
    CalcDelayLen(idx);
}

void Resonator::SetDampPitch(float pitch, int idx) {
    auto& p = paths_[idx];
    p.dampPitch_ = pitch;
    auto freq = PitchToFreq(pitch);
    p.lowpass_.SetCutOffFreq(freq);
    CalcDelayLen(idx);
}

void Resonator::CalcDelayLen(int idx) {
    auto& p = paths_[idx];
    auto pathPitch = p.pitchShift_ + globalPitch_ + p.detune_ / 50.0f;
    if (!p.decaySign_) { // square wave
        pathPitch += 12;
    }
    if (pathPitch > kMaxPitch || pathPitch < kMinPitch) {
        p.disable_ = true;
    }
    else {
        p.disable_ = false;
        p.freq_ = PitchToFreq(pathPitch);
        p.delayLen_ = sampleRate_ / p.freq_;
        auto allpassLen = p.delayLen_ * p.dispersionRatio_;
        p.thrianAllpass_.SetGroupDelay(allpassLen);
        auto realAllpassLen = p.thrianAllpass_.GetPhaseDelay(p.freq_);
        auto realDelayLen = p.delayLen_ - realAllpassLen;
        p.delay_.SetDelay(realDelayLen);
    }
    CalcDecay(idx);
}

void Resonator::SetOutputMix(float mix, int idx) {
    if (mix < -59.5f) {
        outputMix_[idx] = 0.0f;
        return;
    }
    outputMix_[idx] = DbToGain(mix);
}

void Resonator::SetGlobalPitch(float pitch) {
    globalPitch_ = pitch;
    for (int i = 0; i < kNumPath; ++i) {
        CalcDelayLen(i);
    }
}
}
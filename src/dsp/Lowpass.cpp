#include "Lowpass.hpp"
#include <cmath>
#include <numbers>
#include <complex>

namespace mana {

void Lowpass::Init(float sampleRate) {
    sampleRate_ = sampleRate;
    SetLPF1(1000.0f);
}

float Lowpass::Process(float x) {
    auto t = x - a1_ * latch1_ - a2_ * latch2_;
    auto y = t * b0_ + b1_ * latch1_ + b2_ * latch2_;
    latch2_ = latch1_;
    latch1_ = t;
    return y;
}

void Lowpass::SetCutOffFreq(float freq) {
    switch (loopFilterType_) {
    case LoopFilterType::IIR_LPF1:
        SetLPF1(freq);
        break;
    case LoopFilterType::IIR_LPF2:
        SetLPF2(freq);
        break;
    case LoopFilterType::FIR_LPF:
        SetLPF1FIR(freq);
        break;
    }
}

float Lowpass::GetMaxFreq() const {
    return 20000.0f;
}

void Lowpass::SetLPF1(float freq) {
    freq_ = freq;
    if (freq > GetMaxFreq()) {
        b0_ = 1;
        b1_ = 0;
        b2_ = 0;
        a1_ = 0;
        a2_ = 0;
        return;
    }

    float omega = 2 * std::numbers::pi_v<float> * freq / sampleRate_;
    auto k = std::tan(omega / 2);
    b0_ = k / (1 + k);
    b1_ = b0_;
    b2_ = 0;
    a1_ = (k - 1) / (k + 1);
    a2_ = 0;
}

void Lowpass::SetLPF2(float freq) {
    freq_ = freq;
    if (freq > GetMaxFreq()) {
        b0_ = 1;
        b1_ = 0;
        b2_ = 0;
        a1_ = 0;
        a2_ = 0;
        return;
    }

    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto k = std::tan(omega / 2);
    constexpr auto Q = 1.0f / std::numbers::sqrt2_v<float>;
    auto down = k * k * Q + k + Q;
    b0_ = k * k * Q / down;
    b1_ = 2 * b0_;
    b2_ = b0_;
    a1_ = 2 * Q * (k * k - 1) / down;
    a2_ = (k * k * Q - k + Q) / down;
}

void Lowpass::SetLPF1FIR(float freq) {
    freq_ = freq;
    if (freq > GetMaxFreq()) {
        b0_ = 1;
        b1_ = 0;
        b2_ = 0;
        a1_ = 0;
        a2_ = 0;
        return;
    }

    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto cosw = std::cos(omega);

    b0_ = (1 + cosw) / 4;
    b1_ = (1 - cosw) / 2;
    b2_ = b0_;
    a1_ = 0.0f;
    a2_ = 0.0f;
}

float Lowpass::GetPhaseDelay(float freq) const {
    if (freq > GetMaxFreq()) return 0.0f;

    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto z = std::polar(1.0f, omega);
    auto up = b2_ * z * z + b1_ * z + 1.0f;
    auto down = z * z + b1_ * z + b2_;
    auto response = up / down;
    return -std::arg(response) / omega;
}

float Lowpass::GetGroupDelay(float freq) const {
    if (loopFilterType_ == LoopFilterType::FIR_LPF) return 1.0f;

    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    float hReal = b0_ + b1_ * std::cos(omega) + b2_ * std::cos(2 * omega);
    float hImag = b1_ * std::sin(omega) + b2_ * std::sin(2 * omega);
    float hMagnitude = std::sqrt(hReal * hReal + hImag * hImag);

    // 计算相位
    float phase = std::atan2(hImag, hReal);

    // 使用数值方法计算导数
    const float delta = 1e-5; // 小的增量
    float phasePlusDelta = std::atan2(
        b1_ * std::sin(omega + delta) + b2_ * std::sin(2 * (omega + delta)),
        b0_ + b1_ * std::cos(omega + delta) + b2_ * std::cos(2 * (omega + delta))
    );

    float phaseMinusDelta = std::atan2(
        b1_ * std::sin(omega - delta) + b2_ * std::sin(2 * (omega - delta)),
        b0_ + b1_ * std::cos(omega - delta) + b2_ * std::cos(2 * (omega - delta))
    );

    // 计算群延迟
    float dPhase = (phasePlusDelta - phaseMinusDelta) / (2 * delta);
    return -dPhase; // 群延迟为相位导数的负值
}

float Lowpass::GetGainPower2(float freq) const {
    if (freq > GetMaxFreq()) return 1.0f;

    auto omega = 2.0f * std::numbers::pi_v<float> * freq / sampleRate_;
    auto z = std::polar(1.0f, omega);
    auto up = b2_ * z * z + b1_ * z + 1.0f;
    auto down = z * z + b1_ * z + b2_;
    auto response = up / down;
    return std::norm(response);
}

void Lowpass::Panic() {
    latch1_ = 0.0f;
    latch2_ = 0.0f;
}

void Lowpass::SetLoopFilterType(LoopFilterType type) {
    if (loopFilterType_ != type) {
        SetCutOffFreq(freq_);
        loopFilterType_ = type;
    }
}
}
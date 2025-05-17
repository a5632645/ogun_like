#pragma once
#include <span>
#include <array>
#include <atomic>
#include <random>
#include <numbers>
#include "curve_v2.h"
#include "AudioFFT/AudioFFT.h"
#include "quad_osc.hpp"

namespace ogun {

class OgunNote {
public:
    static constexpr size_t kWaveTableSize = 8192;
    static constexpr size_t kFFTSize = kWaveTableSize;
    static constexpr size_t kNumFFTBins = kFFTSize / 2 + 1;
    static constexpr size_t kNumAdjustedBins = kFFTSize / 2;
    static constexpr size_t kMinHarmonicNum = 4;  // 16
    static constexpr size_t kMaxHarmonicNum = 13; // 8192
    static constexpr size_t kDefaultFFTSize = 1024;
    static constexpr size_t kDefaultHarmonicNum = 10; // 1024

    OgunNote();
    void Init(float fs);
    void Process(std::span<float> block);
    void SetFrequency(float freq);
    void SetHarmonicNum(int fft_n);
    void SetPhaseSeed(int seed);
    void SetUseSawSlope(bool use_saw_slope);
    void SetVolume(float db_vol);
    void SetBinChanged() { is_bin_changed_.store(true); }
    mana::CurveV2& GetTimbreAmpCurve() { return timbre_amp_; }
    mana::CurveV2& GetTimbreFormantCurve() { return timbre_formant_; }
private:
    using DynamicWaveTable = std::array<float, kWaveTableSize + 1>;

    void UpdateWaveTable();
    void ProcessAudioSpan(std::span<float> block);
    int CalcAntiAlasingBins();
    void ProcessAudioSpanWithCrossFading(std::span<float> block);
    DynamicWaveTable& GetMainWaveTable() const { return *table_a_ptr_; }
    float ReadWaveTable(DynamicWaveTable& table, float index) const;
    void FillAudioSpanFromWaveTable(float* ptr, int size);

    float sample_rate_{};
    float phase_{};
    float phase_inc_{};
    float phase_inc_mul_{};
    float freq_{};
    float volume_{};
    
    audiofft::AudioFFT fft_;
    DynamicWaveTable table1_{};
    DynamicWaveTable table2_{};
    DynamicWaveTable* table_a_ptr_{};
    DynamicWaveTable* table_b_ptr_{};

    std::atomic<bool> is_bin_changed_{};
    std::array<float, kNumAdjustedBins> bin_phases_{};

    int phase_seed_{};
    std::default_random_engine random_engine_;
    std::uniform_real_distribution<float> random_distribution_{0.0f, std::numbers::pi_v<float> * 2};

    int num_bins_{};
    int fft_n_{};
    int anti_alasing_bins_{};
    bool use_saw_slope_;
    mana::CurveV2 timbre_amp_;
    mana::CurveV2 timbre_formant_;
    mana::CurveV2 decay_map_;

    // 定时改变波表
    int update_counter_{};
    int num_samples_per_update_frame_{};
    float update_rate_{};

    // 波表改变，交叉淡化
    int num_samples_still_cross_fading_{};
    int num_total_cross_fading_samples_{};
    int cross_fading_counter_{};
};

}
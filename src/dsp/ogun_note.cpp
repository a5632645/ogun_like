#include "ogun_note.hpp"
#include <cmath>

static float FastDbToGain(float db) {
    if (db < -95.0f) return 0.0f;
    return std::pow(10.0f, db * 0.05f);
}

namespace ogun {
OgunNote::OgunNote()
    : timbre_amp_(kNumAdjustedBins, mana::CurveV2::CurveInitEnum::kFull)
    , timbre_formant_(kNumAdjustedBins, mana::CurveV2::CurveInitEnum::kFull)
{
}

void OgunNote::Init(float fs) {
    sample_rate_ = fs;
    fft_.init(kFFTSize);
}

void OgunNote::Process(std::span<float> block) {
    UpdateWaveTable();

    float inc = phase_inc_ * phase_inc_mul_;
    for (auto& s : block) {
        phase_ += inc;
        if (phase_ >= static_cast<float>(kWaveTableSize)) {
            phase_ -= kWaveTableSize;
        }

        size_t idx = static_cast<size_t>(phase_);
        size_t next = idx + 1;
        float frac = phase_ - idx;
        s = std::lerp(old_table_[idx], old_table_[next], frac);
    }
}

void OgunNote::SetFrequency(float freq) {
    freq_ = freq;
    phase_inc_ = freq * kWaveTableSize / sample_rate_;

    int aa_bins = CalcAntiAlasingBins();
    if (aa_bins != anti_alasing_bins_) {
        SetBinChanged();
    }
}

void OgunNote::SetHarmonicNum(int fft_n) {
    fft_n_ = fft_n;
    int block_size_ = 1 << fft_n;
    num_bins_ = block_size_ / 2;
    timbre_amp_.SetCurrLineResolution(num_bins_);
    timbre_formant_.SetCurrLineResolution(num_bins_);
    if (fft_n > kDefaultHarmonicNum) {
        // 降低音高获得更多谐波
        // 如果要制作pad，降低音高并将谐波级数设定为 N * (fft_n - kDefaultHarmonicNum + 1)
        int octave_down = fft_n - kDefaultHarmonicNum;
        phase_inc_mul_ = 1.0f;
        while (octave_down--) {
            phase_inc_mul_ *= 0.5f;
        }
    }
    else {
        // 减少谐波个数
        phase_inc_mul_ = 1.0f;
    }
}

void OgunNote::SetPhaseSeed(int seed) {
    phase_seed_ = seed;
    random_engine_.seed(seed);
    for (int i = 0; i < kNumAdjustedBins; ++i) {
        bin_phases_[i] = random_distribution_(random_engine_);
    }
}

void OgunNote::SetUseSawSlope(bool use_saw_slope) {
    use_saw_slope_ = use_saw_slope;
}

void OgunNote::UpdateWaveTable() {
    if (is_bin_changed_.exchange(false)) {
        anti_alasing_bins_ = CalcAntiAlasingBins();
        int num_bins = std::min(anti_alasing_bins_, num_bins_);

        float gain_down = kFFTSize / 4 / std::sqrt(static_cast<float>(num_bins_));
        std::array<float, kNumFFTBins> real{};
        std::array<float, kNumFFTBins> imag{};

        if (!use_saw_slope_) {
            for (size_t i = 0; i < num_bins; ++i) {
                float phase = bin_phases_[i];
                float raw_amp = timbre_amp_.Get(static_cast<int>(i));
                float map_amp = std::lerp(-96.0f, 0.0f, raw_amp);
                float raw_formant = timbre_formant_.Get(static_cast<int>(i));
                float map_formant = std::lerp(-96.0f, 0.0f, raw_formant);
                float amp_db = map_amp + map_formant;
                float amp = FastDbToGain(amp_db);
                float gain = amp * gain_down;
                const size_t idx = i + 1;
                real[idx] = gain * std::cos(phase);
                imag[idx] = gain * std::sin(phase);
            }
        }
        else {
            for (size_t i = 0; i < num_bins; ++i) {
                float phase = bin_phases_[i];
                float raw_amp = timbre_amp_.Get(static_cast<int>(i));
                float map_amp = std::lerp(-96.0f, 0.0f, raw_amp);
                float raw_formant = timbre_formant_.Get(static_cast<int>(i));
                float map_formant = std::lerp(-96.0f, 0.0f, raw_formant);
                float amp_db = map_amp + map_formant;
                float amp = FastDbToGain(amp_db);
                float gain = amp * gain_down / (i + 1);
                const size_t idx = i + 1;
                real[idx] = gain * std::cos(phase);
                imag[idx] = gain * std::sin(phase);
            }
        }

        // 频域补零 -> 时域插值
        std::fill(real.begin() + num_bins, real.end(), 0.0f);
        std::fill(imag.begin() + num_bins, imag.end(), 0.0f);
        fft_.ifft(old_table_.data(), real.data(), imag.data());
        old_table_.back() = old_table_.front();
    }
}

int OgunNote::CalcAntiAlasingBins() {
    float true_freq = freq_ * phase_inc_mul_;
    float max_freq = std::min(sample_rate_ / 2.0f, 20000.0f);
    return static_cast<int>(max_freq / true_freq);
}
}
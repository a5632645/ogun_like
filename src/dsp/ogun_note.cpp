#include "ogun_note.hpp"
#include <cmath>
#include <numbers>

static float PhaseWarp(float x) {
    x /= 2.0 * std::numbers::pi_v<float>;
    x -= std::round(x);
    return x * 2.0f * std::numbers::pi_v<float>;
}

namespace ogun {
OgunNote::OgunNote()
    : timbre_amp_(kNumAdjustedBins, mana::CurveV2::CurveInitEnum::kFull)
    , timbre_formant_(kNumAdjustedBins, mana::CurveV2::CurveInitEnum::kFull)
    , phase_move_map_(kNumAdjustedBins, mana::CurveV2::CurveInitEnum::kNull)
    , decay_map_(kNumAdjustedBins, mana::CurveV2::CurveInitEnum::kFull)
{
    table_a_ptr_ = &table1_;
    table_b_ptr_ = &table2_;
}

void OgunNote::Init(float fs) {
    sample_rate_ = fs;
    fft_.init(kFFTSize);

    update_rate_ = 100.0f;
    num_samples_per_update_frame_ = static_cast<int>(std::round(fs / update_rate_));
    update_rate_ = fs / static_cast<float>(num_samples_per_update_frame_);
    update_counter_ = 0;

    num_total_cross_fading_samples_ = static_cast<int>(fs * 10.0f / 1000.0f);
    if (num_total_cross_fading_samples_ > num_samples_per_update_frame_ / 2) {
        num_total_cross_fading_samples_ = num_samples_per_update_frame_ / 2;
    }
}

void OgunNote::Process(std::span<float> block) {
    int num_still = static_cast<int>(block.size());
    int read_pos = 0;
    for (;num_still != 0;) {
        if (update_counter_ == 0) {
            UpdateWaveTable();
            update_counter_ = num_samples_per_update_frame_;
        }
        int block_size = std::min(num_still, num_samples_per_update_frame_);
        ProcessAudioSpan(block.subspan(read_pos, block_size));
        read_pos += block_size;
        num_still -= block_size;
        update_counter_ -= block_size;
    }
}

void OgunNote::SetFrequency(float freq) {
    freq_ = freq;
    phase_inc_ = freq * kWaveTableSize / sample_rate_;
}

void OgunNote::SetHarmonicNum(int fft_n) {
    fft_n_ = fft_n;
    int block_size_ = 1 << fft_n;
    num_bins_ = block_size_ / 2;
    timbre_amp_.SetCurrLineResolution(num_bins_);
    timbre_formant_.SetCurrLineResolution(num_bins_);
    phase_move_map_.SetCurrLineResolution(num_bins_);
    decay_map_.SetCurrLineResolution(num_bins_);
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

void OgunNote::SetVolume(float db_vol) {
    volume_ = std::pow(10.0f, db_vol / 20.0f);
}

void OgunNote::NoteOn(float pitch) {
    // 初始化decay_map
    random_engine_.seed(randomed_decay_seed_);
    for (int i = 0; i < num_bins_; ++i) {
        randomed_decay_map_[i] = random_gain_distribution_(random_engine_);
    }

    // 初始化pre_decay & timbre_impluse
    float gain_down = kFFTSize / 4 / std::sqrt(static_cast<float>(num_bins_));
    for (int i = 0; i < num_bins_; ++i) {
        float mul = gain_down;
        if (use_saw_slope_) {
            mul /= (i + 1.0f);
        }

        if (fullness_ > 0.0f) { // 直接使用impulse_map
            float timbre_impluse = timbre_amp_.Get(i) * timbre_formant_.Get(i) * mul;
            bin_amps_[i] = timbre_impluse;
        }
        else { // 从impluse_map到decay_map
            float frac = -fullness_;
            float lerp_mul = std::lerp(1.0f, randomed_decay_map_[i], frac);
            float timbre_impluse = timbre_amp_.Get(i) * timbre_formant_.Get(i) * mul * lerp_mul;
            bin_amps_[i] = timbre_impluse;
        }

        // predecay
        float decay_v = randomed_decay_map_[i] * decay_map_.Get(i);
        bin_amps_[i] *= std::pow(decay_v, predecay_ * 4);
    }
}

void OgunNote::UpdateWaveTable()
{
    cross_fading_counter_ = 0;
    num_samples_still_cross_fading_ = num_total_cross_fading_samples_;

    auto& table_to_update = *table_b_ptr_;
    anti_alasing_bins_ = CalcAntiAlasingBins();
    int num_bins = std::min(anti_alasing_bins_, num_bins_);

    std::array<float, kNumFFTBins> real{};
    std::array<float, kNumFFTBins> imag{};

    float e = freq_ / sample_rate_ * std::numbers::pi_v<float>;
    if (phase_move_mul_freq_) {
        for (size_t i = 0; i < num_bins; ++i) {
            bin_phases_[i] += phase_move_map_.Get(static_cast<int>(i)) * phase_move_ * e;
            bin_phases_[i] = PhaseWarp(bin_phases_[i]);
        }
    }
    else {
        for (size_t i = 0; i < num_bins; ++i) {
            bin_phases_[i] += phase_move_map_.Get(static_cast<int>(i)) * phase_move_ * (i + 1.0f) * e;
            bin_phases_[i] = PhaseWarp(bin_phases_[i]);
        }
    }

    for (int i = 0; i < num_bins_; ++i) {
        float lerp_v = fullness_ * 0.5f + 0.5f;
        float decay_v = std::lerp(randomed_decay_map_[i], 1.0f, lerp_v) * decay_map_.Get(i);
        bin_amps_[i] *= decay_v;
    }

    for (int i = 0; i < num_bins; ++i) {
        int idx = i + 1;
        float phase = bin_phases_[i];
        real[idx] = bin_amps_[i] * std::cos(phase);
        imag[idx] = bin_amps_[i] * std::sin(phase);
    }

    // if (!use_saw_slope_) {
    //     for (size_t i = 0; i < num_bins; ++i) {
    //         float phase = bin_phases_[i];
    //         float raw_amp = timbre_amp_.Get(static_cast<int>(i));
    //         float raw_formant = timbre_formant_.Get(static_cast<int>(i));
    //         float amp = raw_amp * raw_formant;
    //         float gain = amp * gain_down;
    //         const size_t idx = i + 1;
    //         real[idx] = gain * std::cos(phase);
    //         imag[idx] = gain * std::sin(phase);
    //     }
    // }
    // else {
    //     for (size_t i = 0; i < num_bins; ++i) {
    //         float phase = bin_phases_[i];
    //         float raw_amp = timbre_amp_.Get(static_cast<int>(i));
    //         float raw_formant = timbre_formant_.Get(static_cast<int>(i));
    //         float amp = raw_amp * raw_formant;
    //         float gain = amp * gain_down / (i + 1);
    //         const size_t idx = i + 1;
    //         real[idx] = gain * std::cos(phase);
    //         imag[idx] = gain * std::sin(phase);
    //     }
    // }

    // 频域补零 -> 时域插值
    std::fill(real.begin() + num_bins, real.end(), 0.0f);
    std::fill(imag.begin() + num_bins, imag.end(), 0.0f);
    fft_.ifft(table_to_update.data(), real.data(), imag.data());
    table_to_update.back() = table_to_update.front();
}

void OgunNote::ProcessAudioSpan(std::span<float> block) {
    if (num_samples_still_cross_fading_ != 0) {
        ProcessAudioSpanWithCrossFading(block);
    }
    else {
        FillAudioSpanFromWaveTable(block.data(), static_cast<int>(block.size()));
    }
}

int OgunNote::CalcAntiAlasingBins() {
    float true_freq = freq_ * phase_inc_mul_;
    float max_freq = std::min(sample_rate_ / 2.0f, 20000.0f);
    return static_cast<int>(max_freq / true_freq);
}

void OgunNote::ProcessAudioSpanWithCrossFading(std::span<float> block) {
    int block_size = static_cast<int>(block.size());
    int process_size = std::min(block_size, num_samples_still_cross_fading_);
    int non_cross_size = block_size - process_size;
    num_samples_still_cross_fading_ -= process_size;

    float inc = phase_inc_ * phase_inc_mul_;
    float inv = 1.0f / num_total_cross_fading_samples_;
    for (int i = 0; i < process_size; ++i) {
        phase_ += inc;
        if (phase_ >= static_cast<float>(kWaveTableSize)) {
            phase_ -= kWaveTableSize;
        }

        float a = ReadWaveTable(*table_a_ptr_, phase_);
        float b = ReadWaveTable(*table_b_ptr_, phase_);
        float p = cross_fading_counter_ * inv;
        ++cross_fading_counter_;
        float v = std::lerp(a, b, p);
        block[i] = v * volume_;
    }

    if (num_samples_still_cross_fading_ == 0) {
        std::swap(table_a_ptr_, table_b_ptr_);
    }

    FillAudioSpanFromWaveTable(block.data() + process_size, non_cross_size);
}

float OgunNote::ReadWaveTable(DynamicWaveTable &table, float index) const {
    size_t idx = static_cast<size_t>(index);
    size_t next = idx + 1;
    float frac = index - idx;
    float s = std::lerp(table[idx], table[next], frac);
    return s;
}

void OgunNote::FillAudioSpanFromWaveTable(float* ptr, int size) {
    auto& main_wavetable = GetMainWaveTable();
    float inc = phase_inc_ * phase_inc_mul_;
    for (int i = 0; i < size; ++i) {
        phase_ += inc;
        if (phase_ >= static_cast<float>(kWaveTableSize)) {
            phase_ -= kWaveTableSize;
        }
        ptr[i] = ReadWaveTable(main_wavetable, phase_) * volume_;
    }
}
}
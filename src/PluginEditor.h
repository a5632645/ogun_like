#pragma once

#include "PluginProcessor.h"
#include <array>
#include "ui/common_curve_editor.h"
#include "ui/vertical_slider.hpp"
#include "ui/toggle_button.hpp"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;

    ui::VerticalSlider freq_slider_;
    ui::VerticalSlider harmonic_num_slider_;
    ui::VerticalSlider phase_seed_slider_;
    ui::VerticalSlider volume_slider_;
    ui::VerticalSlider phase_move_slider_;
    ui::VerticalSlider fullness_slider_;
    ui::VerticalSlider predecay_slider_;

    ui::ToggleButton saw_slope_toggle_;
    ui::ToggleButton phase_move_mul_freq_toggle_;
    juce::TextButton click_;

    juce::ComboBox curve_selecter_;
    mana::CommonCurveEditor curve_editor_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};

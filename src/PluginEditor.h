#pragma once

#include "PluginProcessor.h"
#include <array>
#include "ui/common_curve_editor.h"

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

    juce::Slider freq_slider_;
    std::unique_ptr<juce::SliderParameterAttachment> freq_attach_;
    juce::Slider harmonic_num_slider_;
    std::unique_ptr<juce::SliderParameterAttachment> harmonic_num_attach_;
    juce::Slider phase_seed_slider_;
    std::unique_ptr<juce::SliderParameterAttachment> phase_seed_attach_;

    juce::ComboBox curve_selecter_;
    mana::CommonCurveEditor curve_editor_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};

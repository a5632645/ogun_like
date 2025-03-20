#pragma once

#include "PluginProcessor.h"
#include "ui/CombPath.hpp"
#include "ui/LM_slider.h"

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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    std::vector<std::unique_ptr<ui::CombPath>> paths_;
    juce::TextButton buttonPanic_{ "panic", "panic" };
    juce::TextButton buttonUnit_{ "unit", "unit" };
    juce::TextButton buttonHammond_{ "hammond", "hammond" };
    juce::TextButton buttonHammondSplit_{ "hammond_split", "hammond_split" };
    LMKnob globalPitch_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};

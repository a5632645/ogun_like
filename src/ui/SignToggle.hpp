#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace ui {

class SignToggle : public juce::ToggleButton {
public:
    ~SignToggle() override;
    void ParamLink(juce::AudioProcessorValueTreeState& stateToUse, const juce::String& parameterID);
    void paint(juce::Graphics& g) override;
private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attach_;
};

}
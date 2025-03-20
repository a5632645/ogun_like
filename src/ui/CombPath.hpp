#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "LM_slider.h"
#include "SignToggle.hpp"

namespace ui {

class CombPath : public juce::Component {
public:
    CombPath(juce::AudioProcessorValueTreeState& state, int idx);
    void resized() override;
    void paint(juce::Graphics& g) override;
private:
    LMKnob pitch_;
    LMKnob detune_;
    LMKnob feedback_;
    LMKnob dissonance_;
    LMKnob damp_;
    LMKnob outMix_;
    SignToggle sign_;
};

}

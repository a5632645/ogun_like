#include "SignToggle.hpp"

namespace ui {

SignToggle::~SignToggle() {
    attach_ = nullptr;
}

void SignToggle::ParamLink(juce::AudioProcessorValueTreeState& stateToUse, const juce::String& parameterID) {
    attach_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(stateToUse, parameterID, *this);
}

void SignToggle::paint(juce::Graphics& g) {
    auto b = getLocalBounds();
    
    if (getToggleState()) {
        g.setColour(juce::Colours::orange);
        g.fillRect(b);
        g.setColour(juce::Colours::black);
        g.drawText("+", b, juce::Justification::centred, false);
    }
    else {
        g.setColour(juce::Colours::darkgrey);
        g.fillRect(b);
        g.setColour(juce::Colours::white);
        g.drawText("-", b, juce::Justification::centred, false);
    }

    g.setColour(juce::Colours::black);
    g.drawRect(b);
}

}
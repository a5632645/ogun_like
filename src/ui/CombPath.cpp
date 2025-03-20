#include "CombPath.hpp"

namespace ui {

CombPath::CombPath(juce::AudioProcessorValueTreeState& state, int idx) {
    pitch_.setText("shift");
    pitch_.ParamLink(state, juce::String{"shift"} + juce::String{idx});
    addAndMakeVisible(pitch_);

    detune_.setText("detune");
    detune_.ParamLink(state, juce::String{"detune"} + juce::String{idx});
    addAndMakeVisible(detune_);

    feedback_.setText("decay");
    feedback_.ParamLink(state, juce::String{"decay"} + juce::String{idx});
    addAndMakeVisible(feedback_);

    dissonance_.setText("dispersion");
    dissonance_.ParamLink(state, juce::String{"dispersion"} + juce::String{idx});
    addAndMakeVisible(dissonance_);

    damp_.setText("damp");
    damp_.ParamLink(state, juce::String{"damp"} + juce::String{idx});
    addAndMakeVisible(damp_);

    outMix_.setText("out_mix");
    outMix_.ParamLink(state, juce::String{"out_mix"} + juce::String{idx});
    addAndMakeVisible(outMix_);

    sign_.ParamLink(state, juce::String{"sign"} + juce::String{idx});
    addAndMakeVisible(sign_);
}

void CombPath::resized() {
    auto b = getLocalBounds();
    pitch_.setBounds(b.removeFromLeft(80));
    detune_.setBounds(b.removeFromLeft(80));
    feedback_.setBounds(b.removeFromLeft(80));
    dissonance_.setBounds(b.removeFromLeft(80));
    damp_.setBounds(b.removeFromLeft(80));
    outMix_.setBounds(b.removeFromLeft(80));
    sign_.setBounds(b.withSizeKeepingCentre(24, 24));
}

void CombPath::paint(juce::Graphics& g) {
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds());
}

}
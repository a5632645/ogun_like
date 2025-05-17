#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "param_ids.hpp"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    auto& apvts = *p.value_tree_;

    freq_slider_.BindParameter(apvts, "freq");
    freq_slider_.SetShortName("FREQ");
    addAndMakeVisible(freq_slider_);

    harmonic_num_slider_.BindParameter(apvts, "harmonic_num");
    harmonic_num_slider_.SetShortName("HNUM");
    addAndMakeVisible(harmonic_num_slider_);

    phase_seed_slider_.BindParameter(apvts, "phase_seed");
    phase_seed_slider_.SetShortName("PHASE");
    addAndMakeVisible(phase_seed_slider_);

    volume_slider_.BindParameter(apvts, ogun::id::kVolume);
    volume_slider_.SetShortName("VOL");
    addAndMakeVisible(volume_slider_);

    saw_slope_toggle_.BindParameter(apvts, "saw_slope");
    saw_slope_toggle_.setButtonText("saw");
    addAndMakeVisible(saw_slope_toggle_);
    
    curve_selecter_.addItemList({
        "timbre",
        "formant"
    }, 1);
    curve_selecter_.setSelectedItemIndex(0);
    curve_selecter_.onChange = [this] {
        auto idx = curve_selecter_.getSelectedItemIndex();
        switch (idx) {
        case 0:
            curve_editor_.SetCurve(&processorRef.ogun_note_.GetTimbreAmpCurve());
            break;
        case 1:
            curve_editor_.SetCurve(&processorRef.ogun_note_.GetTimbreFormantCurve());
            break;
        }
    };
    addAndMakeVisible(curve_selecter_);

    addAndMakeVisible(curve_editor_);

    setSize (500, 500);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g) {
    g.fillAll(juce::Colours::grey);
}

void AudioPluginAudioProcessorEditor::resized() {
    auto b = getLocalBounds();
    {
        auto top = b.removeFromTop(100);
        freq_slider_.setBounds(top.removeFromLeft(50));
        harmonic_num_slider_.setBounds(top.removeFromLeft(50));
        phase_seed_slider_.setBounds(top.removeFromLeft(50));
        volume_slider_.setBounds(top.removeFromLeft(50));
        {
            auto tbox = top.removeFromLeft(80);
            saw_slope_toggle_.setBounds(tbox.removeFromTop(50));
        }
    }
    {
        auto top = b.removeFromTop(30);
        curve_selecter_.setBounds(top);
    }
    {
        curve_editor_.setBounds(b);
    }
}

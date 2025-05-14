#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    freq_attach_ = std::make_unique<juce::SliderParameterAttachment>(
        *p.value_tree_->getParameter("freq"),
        freq_slider_
    );
    addAndMakeVisible(freq_slider_);

    harmonic_num_attach_ = std::make_unique<juce::SliderParameterAttachment>(
        *p.value_tree_->getParameter("harmonic_num"),
        harmonic_num_slider_
    );
    addAndMakeVisible(harmonic_num_slider_);

    phase_seed_attach_ = std::make_unique<juce::SliderParameterAttachment>(
        *p.value_tree_->getParameter("phase_seed"),
        phase_seed_slider_
    );
    addAndMakeVisible(phase_seed_slider_);
    
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
    freq_attach_ = nullptr;
    harmonic_num_attach_ = nullptr;
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g) {
    g.fillAll(juce::Colours::grey);
}

void AudioPluginAudioProcessorEditor::resized() {
    auto b = getLocalBounds();
    {
        auto top = b.removeFromTop(25);
        freq_slider_.setBounds(top.removeFromLeft(top.getWidth() / 2));
        harmonic_num_slider_.setBounds(top);
    }
    {
        auto top = b.removeFromTop(25);
        phase_seed_slider_.setBounds(top.removeFromLeft(top.getWidth() / 2));
    }
    {
        auto top = b.removeFromTop(30);
        curve_selecter_.setBounds(top);
    }
    {
        curve_editor_.setBounds(b);
    }
}

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

    phase_move_slider_.BindParameter(apvts, ogun::id::kPhaseMove);
    phase_move_slider_.SetShortName("PHMOVE");
    addAndMakeVisible(phase_move_slider_);

    saw_slope_toggle_.BindParameter(apvts, "saw_slope");
    saw_slope_toggle_.setButtonText("saw");
    addAndMakeVisible(saw_slope_toggle_);

    phase_move_mul_freq_toggle_.BindParameter(apvts, ogun::id::kPhaseMoveMulFreq);
    phase_move_mul_freq_toggle_.setButtonText("pm.mulfreq");
    addAndMakeVisible(phase_move_mul_freq_toggle_);

    fullness_slider_.BindParameter(apvts, ogun::id::kFullness);
    fullness_slider_.SetShortName("FULL");
    addAndMakeVisible(fullness_slider_);

    predecay_slider_.BindParameter(apvts, ogun::id::kPreDecay);
    predecay_slider_.SetShortName("PRE");
    addAndMakeVisible(predecay_slider_);

    click_.setButtonText("click");
    click_.onClick = [this] {
        processorRef.ogun_note_.NoteOn(60.0f);
    };
    addAndMakeVisible(click_);
    
    curve_selecter_.addItemList({
        "timbre",
        "formant",
        "phase_move",
        "decay",
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
        case 2:
            curve_editor_.SetCurve(&processorRef.ogun_note_.GetPhaseMoveCurve());
            break;
        case 3:
            curve_editor_.SetCurve(&processorRef.ogun_note_.GetTimerDecayCurve());
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
        phase_move_slider_.setBounds(top.removeFromLeft(50));
        fullness_slider_.setBounds(top.removeFromLeft(50));
        predecay_slider_.setBounds(top.removeFromLeft(50));
        {
            auto tbox = top.removeFromLeft(80);
            saw_slope_toggle_.setBounds(tbox.removeFromTop(20));
            phase_move_mul_freq_toggle_.setBounds(tbox.removeFromTop(20));
            click_.setBounds(tbox.removeFromTop(20));
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

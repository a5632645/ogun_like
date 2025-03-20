#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    auto& apvt = *processorRef.value_tree_;
    for (int i = 0; i < processorRef.resonator_[0].kNumPath; ++i) {
        auto& it = paths_.emplace_back(std::make_unique<ui::CombPath>(apvt, i));
        addAndMakeVisible(*it);
    }

    buttonPanic_.onClick = [this] {
        processorRef.Panic();
    };
    addAndMakeVisible(buttonPanic_);

    buttonUnit_.onClick = [this] {
        const juce::ScopedLock lock{ processorRef.getCallbackLock() };
        processorRef.resonator_[0].MakeUnit();
        processorRef.resonator_[1].MakeUnit();
    };
    addAndMakeVisible(buttonUnit_);

    buttonHammond_.onClick = [this] {
        const juce::ScopedLock lock{ processorRef.getCallbackLock() };
        processorRef.resonator_[0].MakeHammond();
        processorRef.resonator_[1].MakeHammond();
    };
    addAndMakeVisible(buttonHammond_);

    buttonHammondSplit_.onClick = [this] {
        const juce::ScopedLock lock{ processorRef.getCallbackLock() };
        processorRef.resonator_[0].MakeHammondSplit();
        processorRef.resonator_[1].MakeHammondSplit();
    };
    addAndMakeVisible(buttonHammondSplit_);

    globalPitch_.setText("pitch");
    globalPitch_.ParamLink(apvt, "pitch");
    addAndMakeVisible(globalPitch_);
    
    setSize (520, 350);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{

}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour{ 25, 25, 25 });
}

void AudioPluginAudioProcessorEditor::resized()
{
    auto b = getLocalBounds();
    globalPitch_.setBounds(b.removeFromTop(80));
    for (auto& p : paths_) {
        p->setBounds(b.removeFromTop(60));
    }
    b.removeFromTop(10).withHeight(20);
    buttonPanic_.setBounds(b.removeFromLeft(100));
    buttonUnit_.setBounds(b.removeFromLeft(100));
    buttonHammond_.setBounds(b.removeFromLeft(100));
    buttonHammondSplit_.setBounds(b.removeFromLeft(100));
}

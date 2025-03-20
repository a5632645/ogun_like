#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <ranges>
#include <algorithm>
#include <numeric>
#include <numbers>
#include <cmath>

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    {
        auto p = std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID("pitch", 0), "pitch",
            0, 127, 69
        );
        paramListeners_.Add(p, [this](int v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetGlobalPitch(v);
            resonator_[1].SetGlobalPitch(v);
        });
        layout.add(std::move(p));
    }

    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"shift"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterInt>(juce::ParameterID{ name,0 },
            name,
            -48, 48, 0);
        paramListeners_.Add(p, [this, idx = i] (int v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetPitchShift(v, idx);
            resonator_[1].SetPitchShift(v, idx);
        });
        layout.add(std::move(p));
    }

    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"detune"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name,0 },
            name,
            -50 , 50, 0);
            paramListeners_.Add(p, [this, idx = i] (float v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetDetune(v, idx);
            resonator_[1].SetDetune(v, idx);
        });
        layout.add(std::move(p));
    }
    
    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"decay"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{name,0},
            name,
            1, 10000, 1000);
        paramListeners_.Add(p, [this, idx = i] (float v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetDecay(v, idx); 
            resonator_[1].SetDecay(v, idx); 
        });
        layout.add(std::move(p));
    }
    
    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"dispersion"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name,0 },
            name,
            juce::NormalisableRange<float>(0, 1.0f / mana::ThrianAllpass::kMaxNumAPF, 1.0f / mana::ThrianAllpass::kMaxNumAPF / 256.0f), 0
            );
            paramListeners_.Add(p, [this, idx = i] (float v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetDispersion(v, idx); 
            resonator_[1].SetDispersion(v, idx); 
        });
        layout.add(std::move(p));
    }
    
    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"damp"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name,0 },
                                                             name,
                                                             0 , 138, 110);
        paramListeners_.Add(p, [this, idx = i] (float v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetDampPitch(v, idx); 
            resonator_[1].SetDampPitch(v, idx); 
        });
        layout.add(std::move(p));
    }

    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"out_mix"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ name,0 },
                                                             name,
                                                             -60 , 0, -12);
        paramListeners_.Add(p, [this, idx = i] (float v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetOutputMix(v, idx); 
            resonator_[1].SetOutputMix(v, idx); 
        });
        layout.add(std::move(p));
    }

    for (int i = 0; i < kNumPaths; ++i) {
        auto name = juce::String{"sign"} + juce::String{i};
        auto p = std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ name,0 },
                                                             name, true);
        paramListeners_.Add(p, [this, idx = i] (bool v) {
            const juce::ScopedLock lock{ getCallbackLock() };
            resonator_[0].SetDecaySign(v, idx); 
            resonator_[1].SetDecaySign(v, idx); 
        });
        layout.add(std::move(p));
    }

    value_tree_ = std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, "PARAMETERS", std::move(layout));
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    value_tree_ = nullptr;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (auto& r : resonator_) {
        r.Init(sampleRate);
    }
    paramListeners_.CallAll();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    for (auto i = 0; i < totalNumInputChannels; ++i) {
        auto* channelData = buffer.getWritePointer (i);
        auto numSamples = buffer.getNumSamples();
        auto span = std::span(channelData, numSamples);
        for (auto& sample : span) {
            sample = resonator_[i].Process(sample);
        }
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    
}

void AudioPluginAudioProcessor::Panic() {
    const juce::ScopedLock lock{ getCallbackLock() };
    resonator_[0].Panic();
    resonator_[1].Panic();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

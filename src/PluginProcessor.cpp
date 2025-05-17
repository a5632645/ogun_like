#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "param_ids.hpp"

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
        auto freq = std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{"freq", 1},
            "freq",
            50.0f, 500.0f, 110.0f
        );
        paramListeners_.Add(freq, [this](float v) {
            ogun_note_.SetFrequency(v);
        });
        layout.add(std::move(freq));
    }
    {
        auto harmonic_num = std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{"harmonic_num", 1},
            "harmonic_num",
            ogun::OgunNote::kMinHarmonicNum, ogun::OgunNote::kMaxHarmonicNum, ogun::OgunNote::kDefaultHarmonicNum
        );
        paramListeners_.Add(harmonic_num, [this](int fft_n) {
            ogun_note_.SetHarmonicNum(fft_n);
            ogun_note_.SetBinChanged();
        });
        layout.add(std::move(harmonic_num));
    }
    {
        auto phase_seed = std::make_unique<juce::AudioParameterInt>(
            juce::ParameterID{"phase_seed", 1},
            "phase_seed",
            0, 100, 0
        );
        paramListeners_.Add(phase_seed, [this](int seed) {
            ogun_note_.SetPhaseSeed(seed);
            ogun_note_.SetBinChanged();
        });
        layout.add(std::move(phase_seed));
    }
    {
        auto saw_slope = std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{"saw_slope", 1},
            "saw_slope",
            true
        );
        paramListeners_.Add(saw_slope, [this](bool saw) {
            ogun_note_.SetUseSawSlope(saw);
            ogun_note_.SetBinChanged();
        });
        layout.add(std::move(saw_slope));
    }
    {
        auto volume = std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ogun::id::kVolume, 1},
            "volume",
            -20.0f, 40.0f, 0.0f
        );
        paramListeners_.Add(volume, [this](float vol) {
            ogun_note_.SetVolume(vol);
        });
        layout.add(std::move(volume));
    }

    value_tree_ = std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, "PARAMETERS", std::move(layout));

    ogun_note_.GetTimbreAmpCurve().AddListener(this);
    ogun_note_.GetTimbreFormantCurve().AddListener(this);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    value_tree_ = nullptr;
    ogun_note_.GetTimbreAmpCurve().RemoveListener(this);
    ogun_note_.GetTimbreFormantCurve().RemoveListener(this);
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
    float fs = static_cast<float>(sampleRate);
    ogun_note_.Init(fs);

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
    
    std::span left_block { buffer.getWritePointer(0), static_cast<size_t>(buffer.getNumSamples()) };
    std::span right_block { buffer.getWritePointer(1), static_cast<size_t>(buffer.getNumSamples()) };
    ogun_note_.Process(left_block);
    std::copy(left_block.begin(), left_block.end(), right_block.begin());
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
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

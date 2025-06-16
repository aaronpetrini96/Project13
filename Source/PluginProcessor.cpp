/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

auto getPhaserRateName() {return juce::String("Phaser RateHz");}
auto getPhaserCenterFreqName() {return juce::String("Phaser Center FreqHz");}
auto getPhaserDepthName() {return juce::String("Phaser Depth %");}
auto getPhaserFeedbackName() {return juce::String("Phaser Feedback %");}
auto getPhaserMixName() {return juce::String("Phaser Mix %");}
auto getPhaserBypassName() {return juce::String("Phaser Bypass");}

auto getChorusRateName() {return juce::String("Chorus RateHz");}
auto getChorusDepthName() {return juce::String("Chorus Depth %");}
auto getChorusCenterDelayName() {return juce::String("Chorus Center Delay Ms");}
auto getChorusFeedbackName() {return juce::String("Chorus Feedback %");}
auto getChorusMixName() {return juce::String("Chorus Mix %");}
auto getChorusBypassName() {return juce::String("Chorus Bypass");}

auto getOverdriveSaturationName() {return juce::String("Overdrive Saturation");}
auto getOverdriveBypassName() {return juce::String("Overdrive Bypass");}

auto getLadderFilterChoices()
{
    return juce::StringArray
    {
        "LPF12",  // low-pass  12 dB/octave
        "HPF12",  // high-pass 12 dB/octave
        "BPF12",  // band-pass 12 dB/octave
        "LPF24",  // low-pass  24 dB/octave
        "HPF24",  // high-pass 24 dB/octave
        "BPF24"   // band-pass 24 dB/octave
    };
}
auto getLadderFilterModeName() {return juce::String("Ladder Filter Mode");}
auto getLadderFilterCutoffName() {return juce::String("Ladder Filter Cutoff Hz");}
auto getLadderFilterResonanceName() {return juce::String("Ladder Filter Resonance");}
auto getLadderFilterDriveName() {return juce::String("Ladder Filter Drive");}
auto getLadderFilterBypassName() {return juce::String("Ladder Filter Bypass");}


auto getGeneralFilterChoices()
{
    return juce::StringArray
    {
        "Peak",
        "bandpass",
        "notch",
        "allpass",
    };
}
auto getGeneralFilterModeName() {return juce::String("General Filter Mode");}
auto getGeneralFilterFreqName() {return juce::String("General Filter Freq Hz");}
auto getGeneralFilterQualityName() {return juce::String("General Filter Quality");}
auto getGeneralFilterGainName() {return juce::String("General Filter Gain");}
auto getGeneralFilterBypassName() {return juce::String("General Filter Bypass");}

auto getSelectedTabName() {return juce::String("Selected Tab");}

auto getInputGainName() {return juce::String("Input Gain dB");}
auto getOutputGainName() {return juce::String("Output Gain dB");}

//==============================================================================
Project13AudioProcessor::Project13AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
    for(size_t i = 0; i < static_cast<size_t>(DSP_Option::END_OF_LIST); ++i)
    {
        dspOrder[i] = static_cast<DSP_Option>(i);
    }
    
    restoreDspOrderFifo.push(dspOrder);
    
    //============ FLOAT PARAMS ============
    auto floatParams = std::array
    {
        //      PHASER
        &phaserRatehz,
        &phaserCenterFreqhz,
        &phaserDepthPercent,
        &phaserFeedbackPercent,
        &phaserMixPercent,
        
        //      CHORUS
        &chorusRatehz,
        &chorusDepthPercent,
        &chorusCenterDelayms,
        &chorusFeedbackPercent,
        &chorusMixPercent,
        
        //      OD
        &overdriveSaturation,
        
        //       LADDER FILTER
        &ladderFilterCutoffHz,
        &ladderFilterResonance,
        &ladderFilterDrive,
        
        //        GENERAL FILTER
        &generalFilterFreqHz,
        &generalFilterQuality,
        &generalFilterGain,
        
        //      IN OUT Gain
        &inputGain,
        &outputGain,
    };
    
    auto floatNameFuncs = std::array
    {
        //      PHASER
        &getPhaserRateName,
        &getPhaserCenterFreqName,
        &getPhaserDepthName,
        &getPhaserFeedbackName,
        &getPhaserMixName,
        
        //      CHORUS
        &getChorusRateName,
        &getChorusDepthName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusMixName,
        
        //        OD
        &getOverdriveSaturationName,
        
        //        LADDER FILTER
        &getLadderFilterCutoffName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,
        
        //        GENERAL FILTER
        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName,
        
        //      IN OUT Gain
        &getInputGainName,
        &getOutputGainName,
        
    };
    
    initCachedParams<juce::AudioParameterFloat*>(floatParams, floatNameFuncs);
    
    //============ CHOICE PARAMS ============
    
    auto choiceParams = std::array
    {
        &ladderFilterMode,
        &generalFilterMode,
        
    };
    
    auto choiceNameFuncs = std::array
    {
        &getLadderFilterModeName,
        &getGeneralFilterModeName,
    };
    
    
    initCachedParams<juce::AudioParameterChoice*>(choiceParams, choiceNameFuncs);
    
    //============ BOOL PARAMS ============
    auto bypassParams = std::array
    {
        &phaserBypass,
        &chorusBypass,
        &overdriveBypass,
        &ladderFilterBypass,
        &generalFilterBypass,
    };
    
    auto bypassNameFuncs = std::array
    {
        &getPhaserBypassName,
        &getChorusBypassName,
        &getOverdriveBypassName,
        &getLadderFilterBypassName,
        &getGeneralFilterBypassName,
    };
    
    initCachedParams<juce::AudioParameterBool*>(bypassParams, bypassNameFuncs);
    
    //============ INT PARAMS ============
    auto intParams = std::array
    {
        &selectedTab,
    };
    
    auto intNameFuncs = std::array
    {
        &getSelectedTabName,
    };
    
    initCachedParams<juce::AudioParameterInt*>(intParams, intNameFuncs);
}
Project13AudioProcessor::~Project13AudioProcessor()
{
}

//==============================================================================
const juce::String Project13AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Project13AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Project13AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Project13AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Project13AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Project13AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Project13AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Project13AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Project13AudioProcessor::getProgramName (int index)
{
    return {};
}

void Project13AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Project13AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    
    leftChannel.prepare(spec);
    rightChannel.prepare(spec);
    
    for(auto smoother : getSmoothers())
        smoother->reset(sampleRate, 0.005);
    
    updateSmootherFromParams(1, SmootherUpdateMode::initialize);
    
    spec.numChannels = getTotalNumInputChannels();
    
    inputGainDSP.prepare(spec);
    outputGainDSP.prepare(spec);
}

void Project13AudioProcessor::updateSmootherFromParams(int numSamplesToSkip, SmootherUpdateMode init)
{
    auto paramsNeedingSmoothing = std::vector
    {
        phaserRatehz,
        phaserCenterFreqhz,
        phaserDepthPercent,
        phaserFeedbackPercent,
        phaserMixPercent,
        chorusRatehz,
        chorusDepthPercent,
        chorusCenterDelayms,
        chorusFeedbackPercent,
        chorusMixPercent,
        overdriveSaturation,
        ladderFilterCutoffHz,
        ladderFilterResonance,
        ladderFilterDrive,
        generalFilterFreqHz,
        generalFilterQuality,
        generalFilterGain,
        inputGain,
        outputGain,
    };
    
    auto smoothers = getSmoothers();
    jassert(smoothers.size() == paramsNeedingSmoothing.size());
    
    for (size_t i = 0; i < smoothers.size(); ++i)
    {
        auto smoother = smoothers[i];
        auto param = paramsNeedingSmoothing [i];
        
        if (init == SmootherUpdateMode::initialize)
            smoother->setCurrentAndTargetValue(param->get());
        else
            smoother->setTargetValue(param->get());
        
        smoother->skip(numSamplesToSkip);
    }
}

std::vector<juce::SmoothedValue<float>*> Project13AudioProcessor::getSmoothers()
{
    auto smoothers = std::vector
    {
        &phaserRatehzSmoother,
        &phaserCenterFreqhzSmoother,
        &phaserDepthPercentSmoother,
        &phaserFeedbackPercentSmoother,
        &phaserMixPercentSmoother,
        &chorusRatehzSmoother,
        &chorusDepthPercentSmoother,
        &chorusCenterDelaymsSmoother,
        &chorusFeedbackPercentSmoother,
        &chorusMixPercentSmoother,
        &overdriveSaturationSmoother,
        &ladderFilterCutoffHzSmoother,
        &ladderFilterResonanceSmoother,
        &ladderFilterDriveSmoother,
        &generalFilterFreqHzSmoother,
        &generalFilterQualitySmoother,
        &generalFilterGainSmoother,
        &inputGainSmoother,
        &outputGainSmoother,
    };
    
    return smoothers;
}

void Project13AudioProcessor::MonoChannelDSP::prepare(juce::dsp::ProcessSpec &spec)
{
    jassert(spec.numChannels == 1);
    std::vector<juce::dsp::ProcessorBase*> dsp
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderFilter,
        &generalFilter,
    };
    
    for (auto p : dsp)
    {
        p->prepare(spec);
        p->reset();
    }
    
    overdrive.dsp.setCutoffFrequencyHz(20000.f);
}

void Project13AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Project13AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

juce::AudioProcessorValueTreeState::ParameterLayout Project13AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    const int versionHint = 1;
    
    auto name = getInputGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-18.f, 18.f, 0.1f, 1.f), 0.f, "dB"));
    
    name = getOutputGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-18.f, 18.f, 0.1f, 1.f), 0.f, "dB"));
//  PHASER
    name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f), 0.2f, "Hz"));
    
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f), 5.0f, "%"));
    
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 1000.f, "Hz"));
    
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-100.f, 100.f, 0.1f, 1.f), 0.f, "%"));
    
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.f, 100.f, 0.1f, 1.f), 5.f, "%"));
    
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
//    CHORUS
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f), 0.2f, "Hz"));
    
    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.0f, 100.f, 0.1f, 1.f), 5.f, "%"));
    
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f), 7.f, "ms"));
    
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-100.f, 100.f, 0.1f, 1.f), 0.f, "%"));
    
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.f, 100.f, 0.1f, 1.f), 0.f, "%"));
    
    name = getChorusBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
//    OVERDRIVE
    name = getOverdriveSaturationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(1.f, 100.f, 0.01f, 1.f), 1.f, ""));
    
    name = getOverdriveBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
//    LADDER FILTER
    name = getLadderFilterModeName();
    auto choices = getLadderFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(name, versionHint), name, choices, 0));
    
    name = getLadderFilterCutoffName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(20.f, 20000.f, 0.1f, 1.f), 20000.f, "Hz"));
    
    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.f, 100.f, 0.1f, 1.f), 0.f, "%"));
    
    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(1.f, 100.f, 0.01f, 1.f), 1.f,""));
    
    name = getLadderFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
//    GENERAL FILTER
    name = getGeneralFilterModeName();
    choices = getGeneralFilterChoices();
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID(name, versionHint), name, choices, 0));
    
    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 750.f, "Hz"));
    
    name = getGeneralFilterQualityName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.01f, 10.f, 0.01f, 1.f), 0.707f, "")); // Tutorial Qmax = 100
//    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f, ""));
    
    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f, "dB"));
    
    name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
    name = getSelectedTabName();
    layout.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID(name, versionHint), name, 0, static_cast<int>(DSP_Option::END_OF_LIST) - 1, static_cast<int>(DSP_Option::Chorus)));
    
    return layout;
}

void Project13AudioProcessor::MonoChannelDSP::updateDSPFromParams()
{
    phaser.dsp.setRate(p.phaserRatehzSmoother.getCurrentValue());
    phaser.dsp.setCentreFrequency(p.phaserCenterFreqhzSmoother.getCurrentValue());
    phaser.dsp.setDepth(p.phaserDepthPercentSmoother.getCurrentValue() * 0.01f);
    phaser.dsp.setFeedback(p.phaserFeedbackPercentSmoother.getCurrentValue() * 0.01f);
    phaser.dsp.setMix(p.phaserMixPercentSmoother.getCurrentValue() * 0.01f);
    
    chorus.dsp.setRate(p.chorusRatehzSmoother.getCurrentValue());;
    chorus.dsp.setDepth(p.chorusDepthPercentSmoother.getCurrentValue() * 0.01f);
    chorus.dsp.setCentreDelay(p.chorusCenterDelaymsSmoother.getCurrentValue());
    chorus.dsp.setFeedback(p.chorusFeedbackPercentSmoother.getCurrentValue() * 0.01f);
    chorus.dsp.setMix(p.chorusMixPercentSmoother.getCurrentValue() * 0.01f);
    
    overdrive.dsp.setDrive(p.overdriveSaturationSmoother.getCurrentValue());
    
    ladderFilter.dsp.setMode(static_cast<juce::dsp::LadderFilterMode>(p.ladderFilterMode->getIndex()));
    ladderFilter.dsp.setCutoffFrequencyHz(p.ladderFilterCutoffHzSmoother.getCurrentValue());
    ladderFilter.dsp.setResonance(p.ladderFilterResonanceSmoother.getCurrentValue() * 0.01f);
    ladderFilter.dsp.setDrive(p.ladderFilterDriveSmoother.getCurrentValue());
    
//    UPDATE GENERAL FILTER COEFFs HERE
    auto sampleRate = p.getSampleRate();
    auto genMode = p.generalFilterMode->getIndex();
    auto genHz = p.generalFilterFreqHz->get();
    auto genQ = p.generalFilterQuality->get();
    auto genGain = p.generalFilterGain->get();
    
    bool filterChanged = false;
    filterChanged |= (filterFreq != genHz);
    filterChanged |= (filterQ != genQ);
    filterChanged |= (filterGain != genGain);
    
    auto updatedMode = static_cast<GeneralFilterMode>(genMode);
    filterChanged |= (filterMode != updatedMode);
    
    if (filterChanged)
    {
        filterMode = updatedMode;
        filterFreq = genHz;
        filterQ = genQ;
        filterGain = genGain;
        
        juce::dsp::IIR::Coefficients<float>::Ptr coefficients;
        switch(filterMode)
        {
                
            case GeneralFilterMode::Peak:
                coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, filterFreq, filterQ, juce::Decibels::decibelsToGain(filterGain));
                break;
            case GeneralFilterMode::Bandpass:
                coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, filterFreq, filterQ);
                break;
            case GeneralFilterMode::Notch:
                coefficients = juce::dsp::IIR::Coefficients<float>::makeNotch(sampleRate, filterFreq, filterQ);
                break;
            case GeneralFilterMode::Allpass:
                coefficients = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate, filterFreq, filterQ);
                break;
            case GeneralFilterMode::END_OF_LIST:
                jassertfalse;
                break;
        }
        
        if (coefficients != nullptr)
        {
            *generalFilter.dsp.coefficients = *coefficients;
            generalFilter.reset();
        }
    }
}

std::vector<juce::RangedAudioParameter*> Project13AudioProcessor::getParamsForOption(DSP_Option option)
{
    switch (option)
    {
            
        case DSP_Option::Phase:
        {
            return
            {
                // Phase
                phaserRatehz,
                phaserCenterFreqhz,
                phaserDepthPercent,
                phaserFeedbackPercent,
                phaserMixPercent,
                phaserBypass,
            };
        }
            break;
        case DSP_Option::Chorus:
        {
            return
            {
                // Chorus
                chorusRatehz,
                chorusDepthPercent,
                chorusCenterDelayms,
                chorusFeedbackPercent,
                chorusMixPercent,
                chorusBypass,
            };
        }
        case DSP_Option::Overdrive:
        {
            return
            {
                // OD
                overdriveSaturation,
                overdriveBypass,
            };
        }
        case DSP_Option::LadderFilter:
        {
            return
            {
                // LADDER FILTER
                ladderFilterMode,
                ladderFilterCutoffHz,
                ladderFilterResonance,
                ladderFilterDrive,
                ladderFilterBypass,
            };
        }
        case DSP_Option::GeneralFilter:
        {
            return
            {
                // GENERAL FILTER
                generalFilterMode,
                generalFilterFreqHz,
                generalFilterQuality,
                generalFilterGain,
                generalFilterBypass,
            };
        }
        case DSP_Option::END_OF_LIST:
            break;
    }
    
    jassertfalse;
    return {};

}

void Project13AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    //[DONE]: add APVTS
    //[DONE]: create parameters for all dsp choices
    //[DONE]: update DSP here from audio parameters
    //[DONE]: bypass params
    //[DONE]: update general filter coefficients
    //[DONE]: add smoothers for all params updates
    //[DONE]: in/out gain controls
    //[DONE]: save/load settings
    //[DONE]: save/load dsp order
    //[DONE]: bypass DSP
    //[DONE]: Filters are mono not stereo
    //[DONE]: Drag-to-reorder GUI
    //[DONE]: Snap dropped tabs to correct position
    //[DONE]: hide dragged tab image or stop draggin the tab and constrain dragged image to x axis only
    //[DONE]: Restore Tabs in GUI when loading
    //TODO: save/load presets
    //TODO: GUI desing for each DSP instance?
    //[DONE]: restore selectedTab when windows opens
    //[DONE]: bypass button should toggle rotaryslider enablement
    //[DONE]: fix graphic issue when draggin tab over bypass button
    //[DONE]: add bypass buttons to tabs
    //[DONE]: make selected tab more obvious
    //[DONE]: mouse down on tab should change gui
    //[DONE]: replace vertical sliders with SimpleMBComp sliders
    //[DONE]: replace combo boxes with SimpleMBComp combobox
    //[DONE]: replace bypass buttons with SimpleMBComp buttons
    //[DONE]: Restore selected tab when window opens
    //[DONE]: Restore tab order when window opens
    //[DONE]: Restore tab order when window opens first time (After quit)
    //[DONE]: Restore tabs when closing/opening window (no quit)
    //[DONE]: restore selected tab when closing/opening window (no quit)
    //[DONE]: metering
    //[DONE]: prepare allDSP
    //TODO: wet/dry knob [bonus]
    //TODO: mono & stereo versions [mono is bonus]
    //TODO: modulators [bonus]
    //TODO: thread-safe filtering updating [bonus]
    //TODO: pre/post filtering [bonus]
    //TODO: delay module [bonus]
    

    leftChannel.updateDSPFromParams();
    rightChannel.updateDSPFromParams();
    
    auto newDSPOrder = DSP_Order();
    
    //try to pull
    while (dspOrderFifo.pull(newDSPOrder))
    {
#if VERIFY_BYPASS_FUNCTIONALITY
        jassertfalse;
#endif
    }
    
    if (newDSPOrder != DSP_Order())
        dspOrder = newDSPOrder;
    
    //When plugin is 1st loaded, if gui closed an reopened, the restoreDspOrderFifo is empty.
    //restoreDspOrderFifo is populated when setStateInformation is called. An atomic is used as signal.
    if (guiNeedsLatestDspOrder.compareAndSetBool(false, true))
        restoreDspOrderFifo.push(dspOrder);
    
//    auto block = juce::dsp::AudioBlock<float>(buffer);
//    leftChannel.process(block.getSingleChannelBlock(0), dspOrder);
//    rightChannel.process(block.getSingleChannelBlock(1), dspOrder);
    auto block = juce::dsp::AudioBlock<float>(buffer);
    
    auto preCtx = juce::dsp::ProcessContextReplacing<float>(block);
    
    inputGainSmoother.setTargetValue(inputGain -> get());
    outputGainSmoother.setTargetValue(outputGain -> get());
    inputGainDSP.setGainDecibels(inputGainSmoother.getNextValue());
    inputGainDSP.process(preCtx);
    
    const auto numSamples = buffer.getNumSamples();
    auto samplesRemaining = numSamples;
    auto maxSamplesToProcess = juce::jmin(samplesRemaining, 64);
    
    leftPreRMS.set(buffer.getRMSLevel(0, 0, numSamples));
    rightPreRMS.set(buffer.getRMSLevel(1, 0, numSamples));
    

    size_t startSample = 0;
    while (samplesRemaining > 0)
    {
        /*
         figure out how many samples to process.
         i.e. you have a buffer size of 72.
         the first time the samplesToProcess will be 64, because we set maxSamplesToProcess to 64, and samplesRemaining =72.
         the 2nd time smaplesToProcess will be 8 (72-64).
         */
        
        auto samplesToProcess = juce::jmin(samplesRemaining, maxSamplesToProcess);
        updateSmootherFromParams(samplesToProcess, SmootherUpdateMode::liveInRealTime);
        
        leftChannel.updateDSPFromParams();
        rightChannel.updateDSPFromParams();
        
        //creatre sub block form buffer
        auto subBlock = block.getSubBlock(startSample, samplesToProcess);
        
        // procces
        leftChannel.process(subBlock.getSingleChannelBlock(0), dspOrder);
        rightChannel.process(subBlock.getSingleChannelBlock(1), dspOrder);
        
        startSample += samplesToProcess;
        samplesRemaining -= samplesToProcess;
        
    }
    
    auto postCtx = juce::dsp::ProcessContextReplacing<float>(block);
    outputGainDSP.setGainDecibels(outputGainSmoother.getNextValue());
    outputGainDSP.process(postCtx);
    
    leftPostRMS.set(buffer.getRMSLevel(0, 0, numSamples));
    rightPostRMS.set(buffer.getRMSLevel(1, 0, numSamples));
}



void Project13AudioProcessor::MonoChannelDSP::process(juce::dsp::AudioBlock<float> block, const DSP_Order &dspOrder)
{
    //covert dsporder in array of pointers
    DSP_Pointers dspPointers;
    dspPointers.fill({}); // previously dspPointers.fill(nullptr);
    
    for (size_t i = 0; i < dspOrder.size(); ++i)
    {
        switch (dspOrder[i]) {
            case DSP_Option::Phase:
                dspPointers[i].processor = &phaser;
                dspPointers[i].bypassed = p.phaserBypass->get();
                break;
            case DSP_Option::Chorus:
                dspPointers[i].processor = &chorus;
                dspPointers[i].bypassed = p.chorusBypass->get();
                break;
            case DSP_Option::Overdrive:
                dspPointers[i].processor = &overdrive;
                dspPointers[i].bypassed = p.overdriveBypass->get();
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i].processor = &ladderFilter;
                dspPointers[i].bypassed = p.ladderFilterBypass->get();
                break;
            case DSP_Option::GeneralFilter:
                dspPointers[i].processor = &generalFilter;
                dspPointers[i].bypassed = p.generalFilterBypass->get();
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break;
        }
    }
    
    //now process
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    for (size_t i=0; i < dspPointers.size(); ++i)
    {
        if (dspPointers[i].processor != nullptr)
        {
            juce::ScopedValueSetter<bool> svs (context.isBypassed, dspPointers[i].bypassed);
            
#if VERIFY_BYPASS_FUNCTIONALITY
            if (context.isBypassed)
            {
                jassertfalse;
            }
            
            
#endif
            if (dspPointers[i].processor == &generalFilter)
            {
                continue;
            }
            dspPointers[i].processor->process(context);
        }
    }
}

//==============================================================================
bool Project13AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Project13AudioProcessor::createEditor()
{
    return new Project13AudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================

template<>
struct juce::VariantConverter<Project13AudioProcessor::DSP_Order>
{
    static Project13AudioProcessor::DSP_Order fromVar (const juce::var& v)
    {
        using T = Project13AudioProcessor::DSP_Order;
        T dspOrder;
        
        jassert(v.isBinaryData());
        
        if (v.isBinaryData() == false)
        {
            dspOrder.fill(Project13AudioProcessor::DSP_Option::END_OF_LIST);
        }
        else
        {
            auto mb = *v.getBinaryData();
            juce::MemoryInputStream mis(mb, false);
            std::vector<int> arr;
            while (!mis.isExhausted())
            {
                arr.push_back(mis.readInt());
            }
            
            jassert(arr.size() == dspOrder.size());
            
            for(size_t i = 0; i<dspOrder.size(); ++i)
                dspOrder[i] = static_cast<Project13AudioProcessor::DSP_Option>(arr[i]);
        }
        return dspOrder;
    }
    
    static juce::var toVar(const Project13AudioProcessor::DSP_Order& t)
    {
        juce::MemoryBlock mb;
        //juce MOS uses scoping to complete writing
        {
            juce::MemoryOutputStream mos(mb, false);
            for (const auto& v : t)
                mos.writeInt(static_cast<int>(v));

        }
        return mb;
    }
};


void Project13AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    apvts.state.setProperty("dspOrder", juce::VariantConverter<Project13AudioProcessor::DSP_Order>::toVar(dspOrder), nullptr);
    juce::MemoryOutputStream mos(destData, false);
    apvts.state.writeToStream(mos);
}

void Project13AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        
        if(apvts.state.hasProperty("dspOrder"))
        {
            auto order = juce::VariantConverter<Project13AudioProcessor::DSP_Order>::fromVar(apvts.state.getProperty("dspOrder"));
            dspOrderFifo.push(order);
            restoreDspOrderFifo.push(order);
        }
        DBG(apvts.state.toXmlString());
        
#if VERIFY_BYPASS_FUNCTIONALITY
        juce::Timer::callAfterDelay(1000, [this]()
        {
            DSP_Order order;
            order.fill(DSP_Option::LadderFilter);
            order[0] = DSP_Option::Chorus;
            chorusBypass->setValueNotifyingHost(1.f);
            dspOrderFifo.push(order);
        });
        
#endif
    }
        
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Project13AudioProcessor();
}

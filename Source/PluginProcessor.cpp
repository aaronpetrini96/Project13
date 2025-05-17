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
    dspOrder =
    {
        DSP_Option::Phase,
        DSP_Option::Chorus,
        DSP_Option::Overdrive,
        DSP_Option::LadderFilter,
    };
    
    
    
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
        
    };
    
    for (size_t i = 0 ; i < floatParams.size(); ++i)
    {
        auto ptrToParamPtr = floatParams [i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(floatNameFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);
    }
    
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
    
    for (size_t i = 0 ; i < choiceParams.size(); ++i)
    {
        auto ptrToParamPtr = choiceParams [i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(choiceNameFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);
    }

    
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
    
    for (size_t i {0}; i < bypassParams.size(); ++i)
    {
        auto ptrToParamPtr = bypassParams[i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(bypassNameFuncs[i]()));
        jassert(*ptrToParamPtr != nullptr);
    }
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
    spec.numChannels = getTotalNumInputChannels();
    
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
//  PHASER
    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name,                                                         juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f), 0.2f, "Hz"));
    
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f), 0.05f, "%"));
    
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 1000.f, "Hz"));
    
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f), 0.f, "%"));
    
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 30.f), 0.f, "%"));
    
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
//    CHORUS
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f), 0.2f, "Hz"));
    
    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f), 0.05f, "%"));
    
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f), 7.f, "ms"));
    
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f), 0.f, "%"));
    
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 30.f), 0.f, "%"));
    
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
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.f, 1.f, 0.01f, 1.f), 0.f, ""));
    
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
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f, ""));
    
    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(name, versionHint), name, juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f, "dB"));
    
    name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID(name, versionHint), name, false));
    
    return layout;
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
    //TODO: update general filter corrections
    //TODO: add smoothers for all params updates
    //[DONE]: save/load settings
    //[DONE]: save/load dsp order
    //TODO: Filters are mono not stereo
    //TODO: Drag-to-reorder GUI
    //TODO: GUI desing for each DSP instance?
    //TODO: metering
    //[DONE]: prepare allDSP
    //TODO: wet/dry knob [bonus]
    //TODO: mono & stereo versions [mono is bonus]
    //TODO: modulators [bonus]
    //TODO: thread-safe filtering updating [bonus]
    //TODO: pre/post filtering [bonus]
    //TODO: delay module [bonus]
    
    phaser.dsp.setRate(phaserRatehz->get());
    phaser.dsp.setCentreFrequency(phaserCenterFreqhz->get());
    phaser.dsp.setDepth(phaserDepthPercent->get());
    phaser.dsp.setFeedback(phaserFeedbackPercent->get());
    phaser.dsp.setMix(phaserMixPercent->get());
    
    chorus.dsp.setRate(chorusRatehz->get());
    chorus.dsp.setDepth(chorusDepthPercent->get());
    chorus.dsp.setCentreDelay(chorusCenterDelayms->get());
    chorus.dsp.setFeedback(chorusFeedbackPercent->get());
    chorus.dsp.setMix(chorusMixPercent->get());
    
    overdrive.dsp.setDrive(overdriveSaturation->get());
    
    ladderFilter.dsp.setMode(static_cast<juce::dsp::LadderFilterMode>(ladderFilterMode->getIndex()));
    ladderFilter.dsp.setCutoffFrequencyHz(ladderFilterCutoffHz->get());
    ladderFilter.dsp.setResonance(ladderFilterResonance->get());
    ladderFilter.dsp.setDrive(ladderFilterDrive->get());
    
 
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
    
    //covert dsporder in array of pointers
    DSP_Pointers dspPointers;
    dspPointers.fill({}); // previously dspPointers.fill(nullptr);
    
    for (size_t i = 0; i < dspOrder.size(); ++i)
    {
        switch (dspOrder[i]) {
            case DSP_Option::Phase:
                dspPointers[i].processor = &phaser;
                dspPointers[i].bypassed = phaserBypass->get();
                break;
            case DSP_Option::Chorus:
                dspPointers[i].processor = &chorus;
                dspPointers[i].bypassed = chorusBypass->get();
                break;
            case DSP_Option::Overdrive:
                dspPointers[i].processor = &overdrive;
                dspPointers[i].bypassed = overdriveBypass->get();
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i].processor = &ladderFilter;
                dspPointers[i].bypassed = ladderFilterBypass->get();
                break;
            case DSP_Option::GeneralFilter:
                dspPointers[i].processor = &generalFilter;
                dspPointers[i].bypassed = generalFilterBypass->get();
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break;
        }
    }
    
    //now process
    auto block = juce::dsp::AudioBlock<float>(buffer);
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
//    return new Project13AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
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

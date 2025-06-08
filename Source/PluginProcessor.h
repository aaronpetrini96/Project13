/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fifo.h"
//==============================================================================
/**
*/
class Project13AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Project13AudioProcessor();
    ~Project13AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    enum class DSP_Option
    {
        Phase,
        Chorus,
        Overdrive,
        LadderFilter,
        GeneralFilter,
        END_OF_LIST,
    };
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Settings", createParameterLayout()};
    
    using DSP_Order = std::array<DSP_Option, static_cast<size_t>(DSP_Option::END_OF_LIST)>;
    SimpleMBComp::Fifo<DSP_Order> dspOrderFifo, restoreDspOrderFifo;
    

//  Phase
    juce::AudioParameterFloat* phaserRatehz = nullptr;
    juce::AudioParameterFloat* phaserCenterFreqhz = nullptr;
    juce::AudioParameterFloat* phaserDepthPercent = nullptr;
    juce::AudioParameterFloat* phaserFeedbackPercent = nullptr;
    juce::AudioParameterFloat* phaserMixPercent = nullptr;
    juce::AudioParameterBool*  phaserBypass = nullptr;
//  Chorus
    juce::AudioParameterFloat* chorusRatehz = nullptr;
    juce::AudioParameterFloat* chorusDepthPercent = nullptr;
    juce::AudioParameterFloat* chorusCenterDelayms = nullptr;
    juce::AudioParameterFloat* chorusFeedbackPercent = nullptr;
    juce::AudioParameterFloat* chorusMixPercent = nullptr;
    juce::AudioParameterBool*  chorusBypass = nullptr;
//  OD
    juce::AudioParameterFloat* overdriveSaturation = nullptr;
    juce::AudioParameterBool*  overdriveBypass = nullptr;
//   LADDER FILTER
    juce::AudioParameterChoice* ladderFilterMode = nullptr;
    juce::AudioParameterFloat*  ladderFilterCutoffHz = nullptr;
    juce::AudioParameterFloat*  ladderFilterResonance = nullptr;
    juce::AudioParameterFloat*  ladderFilterDrive = nullptr;
    juce::AudioParameterBool*   ladderFilterBypass = nullptr;
//    GENERAL FILTER
    juce::AudioParameterChoice* generalFilterMode = nullptr;
    juce::AudioParameterFloat*  generalFilterFreqHz = nullptr;
    juce::AudioParameterFloat*  generalFilterQuality = nullptr;
    juce::AudioParameterFloat*  generalFilterGain = nullptr;
    juce::AudioParameterBool*   generalFilterBypass = nullptr;
    
//    SMOOTHED
    //  Phase
    juce::SmoothedValue<float>
    phaserRatehzSmoother,
    phaserCenterFreqhzSmoother,
    phaserDepthPercentSmoother,
    phaserFeedbackPercentSmoother,
    phaserMixPercentSmoother,
    chorusRatehzSmoother,
    chorusDepthPercentSmoother,
    chorusCenterDelaymsSmoother,
    chorusFeedbackPercentSmoother,
    chorusMixPercentSmoother,
    overdriveSaturationSmoother,
    ladderFilterCutoffHzSmoother,
    ladderFilterResonanceSmoother,
    ladderFilterDriveSmoother,
    generalFilterFreqHzSmoother,
    generalFilterQualitySmoother,
    generalFilterGainSmoother;
    
    
    enum class GeneralFilterMode
    {
        Peak,
        Bandpass,
        Notch,
        Allpass,
        END_OF_LIST,
    };
    
    

private:
    
    DSP_Order dspOrder;
    
    template <typename DSP>
    struct DSP_Choice : juce::dsp::ProcessorBase
    {
        void prepare(const juce::dsp::ProcessSpec& spec) override
        {
            dsp.prepare(spec);
        }
        void process(const juce::dsp::ProcessContextReplacing<float>& context) override
        {
            dsp.process(context);
        }
        void reset() override
        {
            dsp.reset();
        }
        
        DSP dsp;
    };
    
    struct MonoChannelDSP
    {
        MonoChannelDSP (Project13AudioProcessor& proc) : p(proc) {}
        
        DSP_Choice<juce::dsp::DelayLine<float>> delay;
        DSP_Choice<juce::dsp::Phaser<float>> phaser;
        DSP_Choice<juce::dsp::Chorus<float>> chorus;
        DSP_Choice<juce::dsp::LadderFilter<float>> overdrive, ladderFilter;
        DSP_Choice<juce::dsp::IIR::Filter<float>> generalFilter;
        
        void prepare(juce::dsp::ProcessSpec& spec);
        
        void updateDSPFromParams();
        
        void process(juce::dsp::AudioBlock<float> block, const DSP_Order& dspOrder);
        
    private:
        Project13AudioProcessor& p;
        GeneralFilterMode filterMode = GeneralFilterMode::END_OF_LIST;
        float filterFreq = 0.f, filterQ = 0.f, filterGain = -100.f; //different values than default to update them when plugin is opened
    };
    
    MonoChannelDSP leftChannel {*this};
    MonoChannelDSP rightChannel {*this};
    
    
    
    struct ProcessState
    {
        juce::dsp::ProcessorBase* processor = nullptr;
        bool bypassed = false;
    };
    
    using DSP_Pointers = std::array<ProcessState, static_cast<size_t>(DSP_Option::END_OF_LIST)>;
    
#define VERIFY_BYPASS_FUNCTIONALITY false
    
    template<typename ParamType, typename Params, typename Funcs>
    void initCachedParams(Params paramsArray, Funcs funcsArray)
    {
        for (size_t i = 0 ; i < paramsArray.size(); ++i)
        {
            auto ptrToParamPtr = paramsArray [i];
            *ptrToParamPtr = dynamic_cast<ParamType>(apvts.getParameter(funcsArray[i]()));
            jassert(*ptrToParamPtr != nullptr);
        }
    }
    
    
    std::vector<juce::SmoothedValue<float>*> getSmoothers();
    
    enum class SmootherUpdateMode
    {
        initialize,
        liveInRealTime
    };
    
    void updateSmootherFromParams(int numSamplesToSkip, SmootherUpdateMode init);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project13AudioProcessor)
};

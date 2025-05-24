/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget
{
    ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) {}
    
    bool isInterestedInDragSource (const SourceDetails& dragSourceDetails) override {return false;}
    void itemDropped(const SourceDetails& dragSourceDetails) override {}
};

struct ExtendedTabBarButton: juce::TabBarButton
{
    
};
//==============================================================================
/**
*/
class Project13AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Project13AudioProcessorEditor (Project13AudioProcessor&);
    ~Project13AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Project13AudioProcessor& audioProcessor;
    
    juce::TextButton dspOrderButton {"dsp Order"};
    
    ExtendedTabbedButtonBar tabbedComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project13AudioProcessorEditor)
};

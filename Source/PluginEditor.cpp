/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GUI/RotarySliderWithLabels.h"
#include "GUI/Utilities.h"
#include "GUI/CustomButtons.h"

static juce::String getNameFromDSPOption(Project13AudioProcessor::DSP_Option option)
{
    switch(option)
    {
        case Project13AudioProcessor::DSP_Option::Phase:
            return "PHASE";
        case Project13AudioProcessor::DSP_Option::Chorus:
            return "CHORUS";
        case Project13AudioProcessor::DSP_Option::Overdrive:
            return "OVERDRIVE";
        case Project13AudioProcessor::DSP_Option::LadderFilter:
            return "LADDERFILTER";
        case Project13AudioProcessor::DSP_Option::GeneralFilter:
            return "GEN FILTER";
        case Project13AudioProcessor::DSP_Option::END_OF_LIST:
            jassertfalse;
    }
    return "NO SELECTION";
}

static Project13AudioProcessor::DSP_Option getDSPOptionFromName(juce::String name)
{
    if(name == "PHASE")
        return Project13AudioProcessor::DSP_Option::Phase;
    if(name == "CHORUS")
        return Project13AudioProcessor::DSP_Option::Chorus;
    if(name == "OVERDRIVE")
        return Project13AudioProcessor::DSP_Option::Overdrive;
    if(name == "LADDERFILTER")
        return Project13AudioProcessor::DSP_Option::LadderFilter;
    if(name == "GEN FILTER")
        return Project13AudioProcessor::DSP_Option::GeneralFilter;
    
    return Project13AudioProcessor::DSP_Option::END_OF_LIST;
}

//==============================================================================
//  ============  EXTENDED TABBED BUTTON BAR ============

ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    auto img = juce::Image(juce::Image::PixelFormat::SingleChannel, 1, 1, true);
    auto gfx = juce::Graphics(img);
    gfx.fillAll(juce::Colours::transparentBlack);
    
    dragImage = juce::ScaledImage(img, 1.0);
}

bool ExtendedTabbedButtonBar::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
    if (dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
        return true;
    return false;
}

void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails &dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragEnter");
    juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);
}

juce::TabBarButton* ExtendedTabbedButtonBar::findDraggedItem (const SourceDetails& dragSourceDetails)
{
    return getTabButton(findDraggedItemIndex(dragSourceDetails));
}

int ExtendedTabbedButtonBar::findDraggedItemIndex (const SourceDetails& dragSourceDetails)
{
    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();
        auto idx = tabs.indexOf(tabButtonBeingDragged);
        return idx;
    }
    
    return -1;
}

struct Comparator
{
    static int compareElements(juce::TabBarButton* first, juce::TabBarButton* second)
    {
        auto firstX = first->getX();
        auto secondX = second->getX();
        if (firstX < secondX)
            return -1;
        else if (firstX == secondX)
            return 0;
        else
            return 1;
    }
};

juce::Array<juce::TabBarButton*> ExtendedTabbedButtonBar::getTabs()
{
    auto numTabs = getNumTabs();
    auto tabs = juce::Array<juce::TabBarButton*>();
    tabs.resize(numTabs);
    for (int i {0}; i < numTabs; ++i)
        tabs.getReference(i) = getTabButton(i);
    
    auto unsorted = tabs;
    Comparator comparator;
    tabs.sort(comparator);
    
    return tabs;
}

void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails &dragSourceDetails)
{
    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();
        
        auto index = findDraggedItemIndex(dragSourceDetails);
        if (index == -1)
        {
            DBG("failed to find tab being dragged in list of tabs");
            jassertfalse;
            return;
        }
        
        //find the tab that tabButtonBeingDragged is colliding with
        //where is it ? left/right
        //if right swap positions
    
        auto previousTabIndex = index -1;
        auto nextTabIndex = index + 1;
        auto previousTab = getTabButton(previousTabIndex);
        auto nextTab = getTabButton(nextTabIndex);
        
        
        auto centerX = tabButtonBeingDragged->getBounds().getCentreX();
        
        if( previousTab == nullptr && nextTab != nullptr)
        {
            //you're 0th pos
            if (centerX > nextTab->getX())
            {
                moveTab(index, nextTabIndex);
            }
        }
        else if(previousTab != nullptr && nextTab == nullptr)
        {
            //you are in last pos
            if (centerX < previousTab -> getX())
            {
                moveTab(index, previousTabIndex);
            }
        }
        else
        {
            //you are in the middle
            if (centerX > nextTab->getX())
            {
                moveTab(index, nextTabIndex);
            }
            else if (centerX < previousTab -> getRight())
            {
                moveTab(index, previousTabIndex);
            }
        }
        
        tabButtonBeingDragged->toFront(true);
    }
}

void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails &dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);
}

void ExtendedTabbedButtonBar::itemDropped(const SourceDetails &dragSourceDetails)
{
    DBG("ExtendedTabbedButtonBar::itemDropped");
//    find the droppped item. Lock position in.
    resized();
    
    auto tabs = getTabs();
    Project13AudioProcessor::DSP_Order newOrder;
    
    jassert(tabs.size() == newOrder.size());
    
    for (size_t i = 0; i < tabs.size(); ++i)
    {
        auto tab = tabs [static_cast<int>(i)];
        if (auto* etbb = dynamic_cast<ExtendedTabBarButton*>(tab))
            newOrder[i] = etbb->getOption();
    }
  
    
    listeners.call([newOrder](Listener& l)
    {
        l.tabOrderChanged(newOrder);
    });
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent &e)
{
    DBG("ExtendedTabbedButtonBar::mouseDown");
    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent))
    {
        auto tabs = getTabs();
        auto idx = tabs.indexOf(tabButtonBeingDragged);
        if (idx != -1)
            setCurrentTabIndex(idx);
        
        startDragging(tabButtonBeingDragged->TabBarButton::getTitle(), tabButtonBeingDragged, dragImage);
    }
}

struct PowerButtonWithParam : PowerButton
{
    PowerButtonWithParam(juce::RangedAudioParameter* p);
    void changeAttachment(juce::RangedAudioParameter* p);
    
private:
    std::unique_ptr<juce::ButtonParameterAttachment> attachment;
};

PowerButtonWithParam::PowerButtonWithParam(juce::RangedAudioParameter* p)
{
    jassert(p != nullptr);
    changeAttachment(p);
}

void PowerButtonWithParam::changeAttachment(juce::RangedAudioParameter* p)
{
    attachment.reset();
    if(p != nullptr)
    {
        attachment = std::make_unique<juce::ButtonParameterAttachment>(*p, *this);
        attachment -> sendInitialUpdate();
    }
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String &tabName, int tabIndex)
{
    auto dspOption = getDSPOptionFromName(tabName);
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this, dspOption);
    etbb -> addMouseListener(this, false);
    
    return etbb.release();
}

void ExtendedTabbedButtonBar::addListener(Listener *l)
{
    listeners.add(l);
}

void ExtendedTabbedButtonBar::removeListener(Listener *l)
{
    listeners.remove(l);
}

void ExtendedTabbedButtonBar::currentTabChanged (int newCurrentTabIndex, [[maybe_unused]]const juce::String& newCurrentTabName)
{
    listeners.call([newCurrentTabIndex](Listener& l)
    {
        l.selectedTabChanged(newCurrentTabIndex);
    });
}

//  ============  HorizontalConstrainer  ============
HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
                                             std::function<juce::Rectangle<int>()> confineeBoundsGetter) :
boundsToConfineToGetter(std::move(confinerBoundsGetter)), boundsOfConfineeGetter(std::move(confineeBoundsGetter))
{
    
}

void HorizontalConstrainer::checkBounds (juce::Rectangle<int>& bounds,
                                         const juce::Rectangle<int>& previousBounds,
                                         const juce::Rectangle<int>& limits,
                                         bool isStretchingTop,
                                         bool isStretchingLeft,
                                         bool isStretchingBottom,
                                         bool isStretchingRight)
{
    bounds.setY(previousBounds.getY()); //getting previous Y as it does no change
//    X needs to be limited
    if (boundsToConfineToGetter != nullptr && boundsOfConfineeGetter != nullptr)
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();
        
        bounds.setX(juce::jlimit(boundsToConfineTo.getX(), boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(), bounds.getX()));
    }
    else
    {
        bounds.setX(juce::jlimit(limits.getX(), limits.getY(), bounds.getX()));
    }
}



//==============================================================================
//  ============  EXTENDED TAB BAR BUTTON  ============

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner, Project13AudioProcessor::DSP_Option dspOption) :
juce::TabBarButton(name, owner),
option(dspOption)
{
    //    horizontal contraining when draggin tabs
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]() {return owner.getLocalBounds();}, [this](){return getLocalBounds();});
    
    constrainer -> setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
}

int ExtendedTabBarButton::getBestTabLength(int depth)
{
    auto bestWidth = getLookAndFeel().getTabButtonBestWidth(*this, depth);
    auto& bar = getTabbedButtonBar();
    
    return juce::jmax(bestWidth, bar.getWidth()/bar.getNumTabs());
    
}

void ExtendedTabBarButton::mouseDown(const juce::MouseEvent &e)
{
    toFront(true);
    dragger.startDraggingComponent (this, e);
    juce::TabBarButton::mouseDown(e);
}

void ExtendedTabBarButton::mouseDrag(const juce::MouseEvent &e)
{
    dragger.dragComponent (this, e, constrainer.get());
}
//==============================================================================

DSP_Gui::DSP_Gui(Project13AudioProcessor& proc) : processor(proc)
{
    
}

void DSP_Gui::resized()
{
    auto bounds = getLocalBounds();
    if (buttons.empty() == false)
    {
        auto buttonArea = bounds.removeFromTop(30);
        auto w = buttonArea.getWidth() / buttons.size();
        
        for (auto& button : buttons)
        {
            button->setBounds(buttonArea.removeFromLeft(static_cast<int>(w)));
        }
    }
    
    if (comboBoxes.empty() == false)
    {
        auto comboBoxArea = bounds.removeFromLeft(bounds.getWidth() / 4);
        auto h = juce::jmin(comboBoxArea.getHeight() / static_cast<int>(comboBoxes.size()), 30);
        
        for (auto& comboBox : comboBoxes)
        {
            comboBox->setBounds(comboBoxArea.removeFromTop(static_cast<int>(h)));
        }
    }
    
    if (sliders.empty() == false)
    {
        auto w = bounds.getWidth() / sliders.size();

        for (auto& slider : sliders)
        {
            slider->setBounds(bounds.removeFromLeft(static_cast<int>(w)));
        }
    }
}

void DSP_Gui::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void DSP_Gui::rebuildInterface (std::vector<juce::RangedAudioParameter*> params)
{
    if (params == currentParams)
    {
        DBG("Interface did no change");
        return;
    }
    DBG("Interface changed");
    currentParams = params;
    
    sliderAttachments.clear();
    comboBoxesAttachments.clear();
    buttonsAttachments.clear();
    
    sliders.clear();
    comboBoxes.clear();
    buttons.clear();
    
    for(size_t i = 0; i < params.size(); ++i)
    {
        auto p = params[i];
        
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(p))
        {
            //combobox
            comboBoxes.push_back(std::make_unique<juce::ComboBox>());
            auto& cb = *comboBoxes.back();
            cb.addItemList(choice->choices, 1);
            
            comboBoxesAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.apvts, p->getName(100), cb));
        }
        else if (auto* toggle = dynamic_cast<juce::AudioParameterBool*>(p))
        {
            //toggle button
            /*
            buttons.push_back(std::make_unique<juce::ToggleButton>("Bypass"));
            auto& btn = *buttons.back();
                        
            buttonsAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.apvts, p->getName(100), btn));
             */
            DBG("DSP_Gui::rebuild interface skipping" << p->getName(100));
        }
        else
        {
            //slider
            
            sliders.push_back(std::make_unique<RotarySliderWithLabels>(p, p->label, p->getName(100)));
            auto& slider = *sliders.back();
            addLabelPairs(slider.labels, *p, p->label);
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
            
            sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.apvts, p->getName(100), slider));
        }
    }
    
    for ( auto& slider : sliders)
        addAndMakeVisible(slider.get());
    for( auto& cb : comboBoxes)
        addAndMakeVisible(cb.get());
    for (auto& b : buttons)
        addAndMakeVisible(b.get());
    
    resized();
}


//==============================================================================
//==============================================================================
Project13AudioProcessorEditor::Project13AudioProcessorEditor (Project13AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&lookAndFeel);
    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGUI);
    
    audioProcessor.guiNeedsLatestDspOrder.set(true);
    
    tabbedComponent.addListener(this);
    startTimerHz(30);
    setSize (768, 400);
}

Project13AudioProcessorEditor::~Project13AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    tabbedComponent.removeListener(this);
}

//==============================================================================
void Project13AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    auto fillMeter = [&] (auto rect, const auto& rmsSource)
    {
        g.setColour(juce::Colours::black);
        g.fillRect(rect);
        
        auto rms = rmsSource.get();
        if (rms > 1.f)
        {
            g.setColour(juce::Colours::red);
            auto lowerLeft = juce::Point<float>(rect.getX(),juce::jmap<float>(juce::Decibels::gainToDecibels(1.f), NEGATIVE_INFINITY, MAX_DECIBELS, rect.getBottom(), rect.getY()));
            
            auto upperRight = juce::Point<float>(rect.getRight(), juce::jmap<float>(juce::Decibels::gainToDecibels(rms), NEGATIVE_INFINITY, MAX_DECIBELS, rect.getBottom(), rect.getY()));
            
            auto overThreshRect = juce::Rectangle<float>(lowerLeft, upperRight);
            g.fillRect(overThreshRect);
        }
        
        rms = juce::jmin<float>(rms, 1.f);
        g.setColour(juce::Colours::green);
        g.fillRect(rect.withY(juce::jmap<float>(juce::Decibels::gainToDecibels(rms), NEGATIVE_INFINITY, MAX_DECIBELS, rect.getBottom(), rect.getY())).withBottom(rect.getBottom()));
    };
    
    
    
    auto drawTicks = [&] (auto rect, auto leftMeterRightEdge, auto rightMeterLeftEdge)
    {
        for (int i = MAX_DECIBELS; i >= NEGATIVE_INFINITY; i -= 12)
        {
            auto y = juce::jmap<int>(i, NEGATIVE_INFINITY, MAX_DECIBELS, rect.getBottom(), rect.getY());
            auto r = juce::Rectangle<int>(rect.getWidth(), fontHeight);
            r.setCentre(rect.getCentreX(), y);
            
            g.setColour(i == 0 ? juce::Colours::white :
                        i > 0 ? juce::Colours::red :
                        juce::Colours::lightsteelblue);
            g.drawFittedText(juce::String(i), r, juce::Justification::centred, 1);
            
            if(i != MAX_DECIBELS && i != NEGATIVE_INFINITY)
            {
                g.drawLine(rect.getX() + tickIndent, y, leftMeterRightEdge - tickIndent, y);
                g.drawLine(rightMeterLeftEdge + tickIndent, y, rect.getRight() - tickIndent, y);
            }
        }
    };
    
    //Lambda draws labels and computes rectangles that form each channel, draws ticks
    auto drawMeter = [&fillMeter, &drawTicks](juce::Rectangle<int> rect, juce::Graphics& g, const juce::Atomic<float>& leftSource, const juce::Atomic<float>& rightSource, const juce::String& label)
    {
        g.setColour(juce::Colours::green);
        g.drawRect(rect);
        rect.reduce(2,2);
        
        g.setColour(juce::Colours::white);
        g.drawText(label, rect.removeFromBottom(fontHeight), juce::Justification::centred);
        
        rect.removeFromTop(fontHeight/2);
        
        const auto meterArea = rect;
        const auto leftChan = rect.removeFromLeft(meterChanWidth);
        const auto rightChan = rect.removeFromRight(meterChanWidth);
        
        fillMeter(leftChan, leftSource);
        fillMeter(rightChan, leftSource);
        drawTicks(meterArea, leftChan.getRight(), rightChan.getX());
    };
    
    auto bounds = getLocalBounds();
    auto preMeterArea = bounds.removeFromLeft(meterWidth);
    auto postMeterArea = bounds.removeFromRight(meterWidth);
    
    drawMeter(preMeterArea, g, audioProcessor.leftPreRMS, audioProcessor.rightPreRMS, "In");
    drawMeter(postMeterArea, g, audioProcessor.leftPostRMS, audioProcessor.rightPostRMS, "Out");
    
}

void Project13AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto leftMeterArea = bounds.removeFromLeft(meterWidth);
    auto rightMeterArea = bounds.removeFromRight(meterWidth);
    juce::ignoreUnused(leftMeterArea, rightMeterArea);
    
    tabbedComponent.setBounds(bounds.removeFromTop(30));
    dspGUI.setBounds(bounds);
}

void Project13AudioProcessorEditor::tabOrderChanged(Project13AudioProcessor::DSP_Order newOrder)
{
    rebuildInterface();
    audioProcessor.dspOrderFifo.push(newOrder);
}

void Project13AudioProcessorEditor::timerCallback()
{
    repaint();
    if (audioProcessor.restoreDspOrderFifo.getNumAvailableForReading() == 0)
        return;
    
    using T = Project13AudioProcessor::DSP_Order;
    T newOrder;
    newOrder.fill(Project13AudioProcessor::DSP_Option::END_OF_LIST);
    auto empty = newOrder;
    while (audioProcessor.restoreDspOrderFifo.pull(newOrder))
    {
        ; //do nothing
    }
    
    if (newOrder != empty) //if u pulled nothing newOrder will be filled with end_of_list
    {
        //don't create tabs if newOrder is filled with END_OF_LIST
        addTabsFromDSPOrder(newOrder);
    }
    
    if (selectedTabAttachment == nullptr)
    {
        selectedTabAttachment = std::make_unique<juce::ParameterAttachment>(*audioProcessor.selectedTab, [this](float tabNum)
        {
            auto newTabNum = static_cast<int>(tabNum);
            if (juce::isPositiveAndBelow(newTabNum, tabbedComponent.getNumTabs()))
            {
                tabbedComponent.setCurrentTabIndex(newTabNum);
            }
            else
                jassertfalse;
        });
        selectedTabAttachment->sendInitialUpdate();
        
        
    }
}

void Project13AudioProcessorEditor::addTabsFromDSPOrder(Project13AudioProcessor::DSP_Order newOrder)
{
    tabbedComponent.clearTabs();
    for (auto v : newOrder)
    {
        tabbedComponent.addTab(getNameFromDSPOption(v), juce::Colours::white, -1);
    }
    
    auto numTabs = tabbedComponent.getNumTabs();
    auto size = tabbedComponent.getHeight();
    
    for (int i = 0; i < numTabs; ++i)
    {
        if (auto tab = tabbedComponent.getTabButton(i))
        {
            auto order = newOrder[i];
            auto params = audioProcessor.getParamsForOption(order);
            for (auto p : params)
            {
                if (auto bypass = dynamic_cast<juce::AudioParameterBool*>(p))
                {
                    if (bypass -> name.containsIgnoreCase("bypass"))
                    {
                        auto pbwp = std::make_unique<PowerButtonWithParam>(bypass);
                        pbwp -> setSize(size, size);
                        tab -> setExtraComponent(pbwp.release(), juce::TabBarButton::ExtraComponentPlacement::beforeText);
                    }
                }
            }
        }
    }
    
    rebuildInterface();
    //if newOrder is the same from before this will do nothing because the order of DSP_Order wont change
    audioProcessor.dspOrderFifo.push(newOrder);
}

void Project13AudioProcessorEditor::rebuildInterface()
{
    auto currentTabIndex = tabbedComponent.getCurrentTabIndex();
    auto currentTab = tabbedComponent.getTabButton(currentTabIndex);
    if (auto etbb = dynamic_cast<ExtendedTabBarButton*>(currentTab))
    {
        auto option = etbb->getOption();
        auto params = audioProcessor.getParamsForOption(option);
        jassert(params.empty() == false);
        dspGUI.rebuildInterface(params);
    }
}

void Project13AudioProcessorEditor::selectedTabChanged(int newCurrentTabIndex)
{
    if (selectedTabAttachment)
    {
        rebuildInterface();
        selectedTabAttachment -> setValueAsCompleteGesture(static_cast<float>(newCurrentTabIndex));
    }
}

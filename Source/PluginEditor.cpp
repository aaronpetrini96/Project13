/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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

juce::Array<juce::TabBarButton*> ExtendedTabbedButtonBar::getTabs()
{
    auto numTabs = getNumTabs();
    auto tabs = juce::Array<juce::TabBarButton*>();
    tabs.resize(numTabs);
    for (int i {0}; i < numTabs; ++i)
        tabs.getReference(i) = getTabButton(i);
    
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
        startDragging(tabButtonBeingDragged->TabBarButton::getTitle(), tabButtonBeingDragged, dragImage);
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
//==============================================================================
Project13AudioProcessorEditor::Project13AudioProcessorEditor (Project13AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{



    addAndMakeVisible(tabbedComponent);
    
    tabbedComponent.addListener(this);
    startTimerHz(30);
    setSize (600, 400);
}

Project13AudioProcessorEditor::~Project13AudioProcessorEditor()
{
    tabbedComponent.removeListener(this);
}

//==============================================================================
void Project13AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void Project13AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    bounds.removeFromTop(10);
    
    tabbedComponent.setBounds(bounds.withHeight(30));
}

void Project13AudioProcessorEditor::tabOrderChanged(Project13AudioProcessor::DSP_Order newOrder)
{
    audioProcessor.dspOrderFifo.push(newOrder);
}

void Project13AudioProcessorEditor::timerCallback()
{
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
}

void Project13AudioProcessorEditor::addTabsFromDSPOrder(Project13AudioProcessor::DSP_Order newOrder)
{
    tabbedComponent.clearTabs();
    for (auto v : newOrder)
    {
        tabbedComponent.addTab(getNameFromDSPOption(v), juce::Colours::white, -1);
    }
    
    //if newOrder is the same from before this will do nothing because the order of DSP_Order wont change
    audioProcessor.dspOrderFifo.push(newOrder);
}

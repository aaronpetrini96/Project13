/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String getDSPOptionName(Project13AudioProcessor::DSP_Option option)
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

//==============================================================================
//  ============  EXTENDED TABBED BUTTON BAR ============

ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    
}

bool ExtendedTabbedButtonBar::isInterestedInDragSource(const SourceDetails &dragSourceDetails)
{
//    if (auto tabBarBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
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
        
        if( previousTab == nullptr && nextTab != nullptr)
        {
            //you're 0th pos
            if (tabButtonBeingDragged->getX() > nextTab->getBounds().getCentreX())
            {
                moveTab(index, nextTabIndex);
            }
        }
        else if(previousTab != nullptr && nextTab == nullptr)
        {
            //you are in last pos
            if (tabButtonBeingDragged->getX() < previousTab -> getBounds().getCentreX())
            {
                moveTab(index, previousTabIndex);
            }
        }
        else
        {
            //you are in the middle
            if (tabButtonBeingDragged->getX() > nextTab->getBounds().getCentreX())
            {
                moveTab(index, nextTabIndex);
            }
            else if (tabButtonBeingDragged->getX() < previousTab -> getBounds().getCentreX())
            {
                moveTab(index, previousTabIndex);
            }
        }
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
}

void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent &e)
{
    DBG("ExtendedTabbedButtonBar::mouseDown");
    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.originalComponent))
    {
        startDragging(tabButtonBeingDragged->TabBarButton::getTitle(), tabButtonBeingDragged);
    }
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String &tabName, int tabIndex)
{
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this);
    etbb -> addMouseListener(this, false);
    
    return etbb.release();
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

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner) :
juce::TabBarButton(name, owner)
{
    //    horizontal contraining when draggin tabs
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]() {return owner.getLocalBounds();}, [this](){return getLocalBounds();});
    
    constrainer -> setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
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
    dspOrderButton.onClick = [this]()
    {
        juce::Random r;
        Project13AudioProcessor::DSP_Order dspOrder;
        
        auto range = juce::Range<int>(static_cast<int>(Project13AudioProcessor::DSP_Option::Phase), static_cast<int>(Project13AudioProcessor::DSP_Option::END_OF_LIST));
        
        
        tabbedComponent.clearTabs();
        for(auto& v : dspOrder)
        {
            auto entry = r.nextInt(range);
            v = static_cast<Project13AudioProcessor::DSP_Option>(entry);
            tabbedComponent.addTab(getDSPOptionName(v), juce::Colours::white, -1);
        }
        
        DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
//        jassertfalse;
        
        audioProcessor.dspOrderFifo.push(dspOrder);
    };

    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);
    setSize (400, 300);
}

Project13AudioProcessorEditor::~Project13AudioProcessorEditor()
{
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
    dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150, 30));
    bounds.removeFromTop(10);
    
    tabbedComponent.setBounds(bounds.withHeight(30));
}

/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Custom UI styling for the Envelope Generator plugin

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

// Color definitions
const juce::Colour CustomLookAndFeel::backgroundColour = juce::Colour(0xFF1A1A2E);
const juce::Colour CustomLookAndFeel::panelColour = juce::Colour(0xFF16213E);
const juce::Colour CustomLookAndFeel::accentColour = juce::Colour(0xFF0F4C75);
const juce::Colour CustomLookAndFeel::stepActiveColour = juce::Colour(0xFF3282B8);
const juce::Colour CustomLookAndFeel::stepInactiveColour = juce::Colour(0xFF2D2D44);
const juce::Colour CustomLookAndFeel::stepPlayingColour = juce::Colour(0xFF00D4FF);
const juce::Colour CustomLookAndFeel::textColour = juce::Colour(0xFFBBE1FA);

CustomLookAndFeel::CustomLookAndFeel()
{
    // Set default colours
    setColour(juce::ResizableWindow::backgroundColourId, backgroundColour);
    setColour(juce::Slider::thumbColourId, accentColour);
    setColour(juce::Slider::rotarySliderFillColourId, stepActiveColour);
    setColour(juce::Slider::rotarySliderOutlineColourId, stepInactiveColour);
    setColour(juce::ComboBox::backgroundColourId, panelColour);
    setColour(juce::ComboBox::textColourId, textColour);
    setColour(juce::ComboBox::outlineColourId, accentColour);
    setColour(juce::ComboBox::arrowColourId, textColour);
    setColour(juce::PopupMenu::backgroundColourId, panelColour);
    setColour(juce::PopupMenu::textColourId, textColour);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accentColour);
    setColour(juce::Label::textColourId, textColour);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool /*shouldDrawButtonAsDown*/)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    
    // Determine color based on state
    juce::Colour fillColour;
    if (button.getToggleState())
    {
        fillColour = stepActiveColour;
    }
    else
    {
        fillColour = stepInactiveColour;
    }
    
    // Highlight on hover
    if (shouldDrawButtonAsHighlighted)
    {
        fillColour = fillColour.brighter(0.2f);
    }
    
    // Draw rounded rectangle
    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // Draw border
    g.setColour(accentColour);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = juce::jmin(6.0f, radius * 0.2f);
    auto arcRadius = radius - lineW * 0.5f;

    auto centre = bounds.getCentre();

    // Background arc
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(stepInactiveColour);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

    // Value arc
    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                               rotaryStartAngle, toAngle, true);
        g.setColour(stepActiveColour);
        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }

    // Thumb/indicator
    juce::Point<float> thumbPoint(centre.x + arcRadius * std::cos(toAngle - juce::MathConstants<float>::halfPi),
                                   centre.y + arcRadius * std::sin(toAngle - juce::MathConstants<float>::halfPi));

    g.setColour(textColour);
    g.fillEllipse(juce::Rectangle<float>(lineW * 1.5f, lineW * 1.5f).withCentre(thumbPoint));
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                      int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                      juce::ComboBox& box)
{
    auto cornerSize = 4.0f;
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(0.5f);

    g.setColour(panelColour);
    g.fillRoundedRectangle(bounds, cornerSize);

    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

    // Draw arrow
    auto arrowZone = juce::Rectangle<int>(width - 20, 0, 15, height).toFloat();
    juce::Path arrow;
    arrow.addTriangle(arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f,
                      arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f,
                      arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);

    g.setColour(box.findColour(juce::ComboBox::arrowColourId));
    g.fillPath(arrow);
}

#pragma once

#include "PluginProcessor.h"

class HeadturnerHatAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    explicit HeadturnerHatAudioProcessorEditor (HeadturnerHatAudioProcessor&);
    ~HeadturnerHatAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ModeButton final : public juce::TextButton
    {
    public:
        using juce::TextButton::TextButton;

        void paintButton (juce::Graphics& g, bool hovered, bool pressed) override
        {
            const auto area = getLocalBounds().toFloat();
            auto fill = juce::Colour::fromRGB (31, 35, 28);

            if (getToggleState())
                fill = juce::Colour::fromRGB (214, 255, 79);
            else if (pressed)
                fill = juce::Colour::fromRGB (76, 82, 62);
            else if (hovered)
                fill = juce::Colour::fromRGB (50, 56, 44);

            g.setColour (fill);
            g.fillRoundedRectangle (area, 18.0f);
            g.setColour (juce::Colour::fromRGBA (255, 255, 255, 18));
            g.drawRoundedRectangle (area.reduced (0.5f), 18.0f, 1.0f);

            g.setColour (getToggleState() ? juce::Colours::black : juce::Colour::fromRGB (231, 227, 205));
            g.setFont (juce::FontOptions ("Chalkboard SE", 15.0f, juce::Font::bold));
            g.drawFittedText (getButtonText(), getLocalBounds(), juce::Justification::centred, 1);
        }
    };

    class HatSlider final : public juce::Slider
    {
    public:
        HatSlider()
        {
            setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        }
    };

    class HatLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                               juce::Slider&) override
        {
            const auto area = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                      static_cast<float> (width), static_cast<float> (height)).reduced (8.0f);
            const auto radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.5f;
            const auto centre = area.getCentre();
            const auto angle = juce::jmap (sliderPosProportional, 0.0f, 1.0f, rotaryStartAngle, rotaryEndAngle);

            g.setColour (juce::Colour::fromRGB (24, 26, 22));
            g.fillEllipse (area);

            g.setColour (juce::Colour::fromRGBA (255, 255, 255, 16));
            g.drawEllipse (area, 1.0f);

            juce::Path arc;
            arc.addCentredArc (centre.x, centre.y, radius - 6.0f, radius - 6.0f, 0.0f,
                               rotaryStartAngle, rotaryEndAngle, true);

            g.setColour (juce::Colour::fromRGBA (213, 255, 77, 30));
            g.strokePath (arc, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            juce::Path valueArc;
            valueArc.addCentredArc (centre.x, centre.y, radius - 6.0f, radius - 6.0f, 0.0f,
                                    rotaryStartAngle, angle, true);
            g.setColour (juce::Colour::fromRGB (213, 255, 77));
            g.strokePath (valueArc, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            juce::Path needle;
            needle.addRoundedRectangle (-2.5f, -radius + 18.0f, 5.0f, radius * 0.56f, 2.0f);

            g.setColour (juce::Colour::fromRGB (255, 123, 72));
            g.fillPath (needle, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

            g.setColour (juce::Colour::fromRGB (232, 228, 208));
            g.fillEllipse (centre.x - 5.0f, centre.y - 5.0f, 10.0f, 10.0f);
        }
    };

    void configureSlider (HatSlider& slider, juce::Label& title, juce::Label& value, const juce::String& text);
    void configureActionButton (juce::TextButton& button, const juce::String& text);
    void refreshValueLabels();
    void timerCallback() override;
    void updateModeButtons();
    juce::File getPresetDirectory() const;
    juce::String sanitisePresetName (juce::String name) const;
    void saveCurrentPreset();
    void loadPreset();

    HeadturnerHatAudioProcessor& audioProcessor;
    HatLookAndFeel lookAndFeel;

    ModeButton closedButton { "Closed" };
    ModeButton openButton { "Open" };
    juce::TextEditor presetNameEditor;
    juce::TextButton savePresetButton { "Save" };
    juce::TextButton loadPresetButton { "Load" };

    HatSlider decaySlider;
    HatSlider toneSlider;
    HatSlider airSlider;
    HatSlider metalSlider;
    HatSlider driveSlider;

    juce::Label decayLabel;
    juce::Label toneLabel;
    juce::Label airLabel;
    juce::Label metalLabel;
    juce::Label driveLabel;

    juce::Label decayValue;
    juce::Label toneValue;
    juce::Label airValue;
    juce::Label metalValue;
    juce::Label driveValue;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> toneAttachment;
    std::unique_ptr<SliderAttachment> airAttachment;
    std::unique_ptr<SliderAttachment> metalAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeadturnerHatAudioProcessorEditor)
};

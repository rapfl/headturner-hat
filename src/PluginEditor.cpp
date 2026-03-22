#include "PluginEditor.h"

HeadturnerHatAudioProcessorEditor::HeadturnerHatAudioProcessorEditor (HeadturnerHatAudioProcessor& audioProcessorIn)
    : AudioProcessorEditor (&audioProcessorIn), audioProcessor (audioProcessorIn)
{
    setLookAndFeel (&lookAndFeel);
    setSize (840, 560);

    closedButton.setClickingTogglesState (true);
    openButton.setClickingTogglesState (true);
    closedButton.setRadioGroupId (1001);
    openButton.setRadioGroupId (1001);

    addAndMakeVisible (closedButton);
    addAndMakeVisible (openButton);
    addAndMakeVisible (presetNameEditor);
    configureActionButton (savePresetButton, "Save");
    configureActionButton (loadPresetButton, "Load");

    presetNameEditor.setText ("Factory Rough", juce::dontSendNotification);
    presetNameEditor.setInputRestrictions (40, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_");
    presetNameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour::fromRGB (18, 21, 17));
    presetNameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour::fromRGBA (213, 255, 77, 80));
    presetNameEditor.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour::fromRGB (213, 255, 77));
    presetNameEditor.setColour (juce::TextEditor::textColourId, juce::Colour::fromRGB (236, 231, 208));
    presetNameEditor.setColour (juce::TextEditor::highlightColourId, juce::Colour::fromRGBA (213, 255, 77, 80));
    presetNameEditor.setJustification (juce::Justification::centredLeft);
    presetNameEditor.setFont (juce::FontOptions ("Chalkboard SE", 15.0f, juce::Font::plain));
    presetNameEditor.setTextToShowWhenEmpty ("Preset name", juce::Colour::fromRGBA (236, 231, 208, 100));

    configureSlider (decaySlider, decayLabel, decayValue, "Decay");
    configureSlider (toneSlider, toneLabel, toneValue, "Tone");
    configureSlider (airSlider, airLabel, airValue, "Air");
    configureSlider (metalSlider, metalLabel, metalValue, "Metal");
    configureSlider (driveSlider, driveLabel, driveValue, "Drive");

    auto& state = audioProcessor.getValueTreeState();
    decayAttachment = std::make_unique<SliderAttachment> (state, "decay", decaySlider);
    toneAttachment = std::make_unique<SliderAttachment> (state, "tone", toneSlider);
    airAttachment = std::make_unique<SliderAttachment> (state, "air", airSlider);
    metalAttachment = std::make_unique<SliderAttachment> (state, "metal", metalSlider);
    driveAttachment = std::make_unique<SliderAttachment> (state, "drive", driveSlider);

    if (auto* openParam = state.getParameter ("open"))
    {
        closedButton.onClick = [openParam]
        {
            openParam->beginChangeGesture();
            openParam->setValueNotifyingHost (0.0f);
            openParam->endChangeGesture();
        };

        openButton.onClick = [openParam]
        {
            openParam->beginChangeGesture();
            openParam->setValueNotifyingHost (1.0f);
            openParam->endChangeGesture();
        };
    }

    const auto updateLabels = [this]
    {
        refreshValueLabels();
    };

    decaySlider.onValueChange = updateLabels;
    toneSlider.onValueChange = updateLabels;
    airSlider.onValueChange = updateLabels;
    metalSlider.onValueChange = updateLabels;
    driveSlider.onValueChange = updateLabels;

    savePresetButton.onClick = [this] { saveCurrentPreset(); };
    loadPresetButton.onClick = [this] { loadPreset(); };

    updateModeButtons();
    refreshValueLabels();
    startTimerHz (20);
}

void HeadturnerHatAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bg (juce::Colour::fromRGB (8, 11, 9), 0.0f, 0.0f,
                             juce::Colour::fromRGB (3, 3, 3), bounds.getRight(), bounds.getBottom(), false);
    bg.addColour (0.36, juce::Colour::fromRGB (18, 20, 15));
    g.setGradientFill (bg);
    g.fillAll();

    g.setGradientFill (juce::ColourGradient (juce::Colour::fromRGBA (213, 255, 77, 24), 80.0f, 80.0f,
                                             juce::Colour::fromRGBA (255, 123, 72, 0), bounds.getRight(), bounds.getBottom(), true));
    g.fillEllipse (40.0f, 20.0f, 320.0f, 220.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour::fromRGBA (255, 123, 72, 28), bounds.getRight() - 220.0f, 80.0f,
                                             juce::Colour::fromRGBA (255, 123, 72, 0), bounds.getRight(), 260.0f, true));
    g.fillEllipse (bounds.getRight() - 280.0f, 40.0f, 240.0f, 180.0f);

    g.setColour (juce::Colour::fromRGBA (213, 255, 77, 18));
    for (int x = 0; x < getWidth(); x += 30)
        g.drawVerticalLine (x, 0.0f, static_cast<float> (getHeight()));

    for (int y = 0; y < getHeight(); y += 24)
        g.drawHorizontalLine (y, 0.0f, static_cast<float> (getWidth()));

    auto panel = bounds.reduced (18.0f);
    g.setColour (juce::Colour::fromRGBA (12, 14, 12, 228));
    g.fillRoundedRectangle (panel, 34.0f);
    g.setColour (juce::Colour::fromRGBA (213, 255, 77, 56));
    g.drawRoundedRectangle (panel, 34.0f, 1.1f);

    auto innerPanel = panel.reduced (14.0f);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 10));
    g.drawRoundedRectangle (innerPanel, 24.0f, 1.0f);

    auto top = panel.removeFromTop (132.0f);

    g.setColour (juce::Colour::fromRGB (213, 255, 77));
    g.setFont (juce::FontOptions ("Impact", 18.0f, juce::Font::plain));
    g.drawText ("ANALOG-STYLE TECHNO HAT SYNTH", top.removeFromTop (24.0f).toNearestInt(), juce::Justification::centredLeft);

    g.setColour (juce::Colour::fromRGB (242, 238, 218));
    g.setFont (juce::FontOptions ("Times New Roman", 64.0f, juce::Font::bold));
    g.drawText ("HEADTURNER HAT", top.removeFromTop (70.0f).toNearestInt(), juce::Justification::centredLeft);

    g.setColour (juce::Colour::fromRGBA (242, 238, 218, 168));
    g.setFont (juce::FontOptions ("Chalkboard SE", 16.0f, juce::Font::plain));
    g.drawFittedText ("Noise-first club hat with a controllable metal layer for glassy ticks or rougher analog bite.",
                      top.toNearestInt(), juce::Justification::centredLeft, 1);

    auto heroBadge = juce::Rectangle<float> (bounds.getRight() - 250.0f, 48.0f, 178.0f, 42.0f);
    g.setColour (juce::Colour::fromRGBA (255, 123, 72, 205));
    g.fillRoundedRectangle (heroBadge, 20.0f);
    g.setColour (juce::Colours::black);
    g.setFont (juce::FontOptions ("Impact", 17.0f, juce::Font::plain));
    g.drawFittedText ("CLUB PRESSURE", heroBadge.toNearestInt(), juce::Justification::centred, 1);

    const auto presetArea = juce::Rectangle<float> (68.0f, 166.0f, static_cast<float> (getWidth() - 136), 56.0f);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 18));
    g.fillRoundedRectangle (presetArea, 22.0f);
    g.setColour (juce::Colour::fromRGBA (213, 255, 77, 48));
    g.drawRoundedRectangle (presetArea, 22.0f, 1.0f);

    auto legendArea = juce::Rectangle<float> (68.0f, 244.0f, static_cast<float> (getWidth() - 136), 34.0f);
    const std::array<juce::String, 5> legends { "ROLL", "CUT", "SPRAY", "BODY", "BURN" };
    auto legendBounds = legendArea.toNearestInt();
    const auto segment = legendBounds.getWidth() / static_cast<int> (legends.size());
    g.setFont (juce::FontOptions ("Impact", 14.0f, juce::Font::plain));

    for (size_t i = 0; i < legends.size(); ++i)
    {
        auto chip = legendBounds.removeFromLeft (segment).reduced (8, 2).toFloat();
        g.setColour (juce::Colour::fromRGBA (255, 255, 255, 12));
        g.fillRoundedRectangle (chip, 12.0f);
        g.setColour (i == 4 ? juce::Colour::fromRGB (255, 123, 72) : juce::Colour::fromRGB (213, 255, 77));
        g.drawFittedText (legends[i], chip.toNearestInt(), juce::Justification::centred, 1);
    }

    auto footer = bounds.reduced (36.0f).removeFromBottom (44.0f);
    g.setColour (juce::Colour::fromRGBA (255, 255, 255, 132));
    g.setFont (juce::FontOptions ("Chalkboard SE", 13.0f, juce::Font::plain));

    const auto isOpen = openButton.getToggleState();
    const auto modeText = isOpen
        ? "Open mode breathes longer for offbeat lifts and rougher transitions."
        : "Closed mode stays clipped and fast for rolling peak-time grooves.";

    g.drawFittedText (modeText, footer.toNearestInt(), juce::Justification::centredLeft, 1);
}

void HeadturnerHatAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (36, 32);
    bounds.removeFromTop (132);

    auto presetRow = bounds.removeFromTop (64);
    presetNameEditor.setBounds (presetRow.removeFromLeft (presetRow.getWidth() - 200).reduced (6, 10));
    savePresetButton.setBounds (presetRow.removeFromLeft (94).reduced (6, 10));
    loadPresetButton.setBounds (presetRow.removeFromLeft (94).reduced (6, 10));

    bounds.removeFromTop (38);
    auto modeArea = bounds.removeFromTop (62);
    closedButton.setBounds (modeArea.removeFromLeft (130).reduced (0, 6));
    modeArea.removeFromLeft (12);
    openButton.setBounds (modeArea.removeFromLeft (130).reduced (0, 6));

    bounds.removeFromTop (6);
    auto controls = bounds.removeFromTop (250);
    const auto width = controls.getWidth() / 5;

    auto decayArea = controls.removeFromLeft (width);
    auto toneArea = controls.removeFromLeft (width);
    auto airArea = controls.removeFromLeft (width);
    auto metalArea = controls.removeFromLeft (width);
    auto driveArea = controls;

    auto layoutDial = [] (juce::Rectangle<int> area, juce::Slider& slider, juce::Label& title, juce::Label& value)
    {
        title.setBounds (area.removeFromTop (24));
        slider.setBounds (area.removeFromTop (170));
        value.setBounds (area.removeFromTop (24));
    };

    layoutDial (decayArea, decaySlider, decayLabel, decayValue);
    layoutDial (toneArea, toneSlider, toneLabel, toneValue);
    layoutDial (airArea, airSlider, airLabel, airValue);
    layoutDial (metalArea, metalSlider, metalLabel, metalValue);
    layoutDial (driveArea, driveSlider, driveLabel, driveValue);
}

void HeadturnerHatAudioProcessorEditor::configureSlider (HatSlider& slider, juce::Label& title, juce::Label& value, const juce::String& text)
{
    addAndMakeVisible (slider);
    addAndMakeVisible (title);
    addAndMakeVisible (value);

    title.setText (text.toUpperCase(), juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centred);
    title.setColour (juce::Label::textColourId, juce::Colour::fromRGB (213, 255, 77));
    title.setFont (juce::FontOptions ("Impact", 16.0f, juce::Font::plain));

    value.setJustificationType (juce::Justification::centred);
    value.setColour (juce::Label::textColourId, juce::Colour::fromRGB (236, 231, 208));
    value.setFont (juce::FontOptions ("Chalkboard SE", 14.0f, juce::Font::plain));
}

void HeadturnerHatAudioProcessorEditor::configureActionButton (juce::TextButton& button, const juce::String& text)
{
    addAndMakeVisible (button);
    button.setButtonText (text.toUpperCase());
    button.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (31, 35, 28));
    button.setColour (juce::TextButton::buttonOnColourId, juce::Colour::fromRGB (213, 255, 77));
    button.setColour (juce::TextButton::textColourOffId, juce::Colour::fromRGB (236, 231, 208));
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    button.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (24, 28, 22));
}

void HeadturnerHatAudioProcessorEditor::refreshValueLabels()
{
    const auto decaySeconds = (0.04 + decaySlider.getValue() * 0.42) * (openButton.getToggleState() ? 2.2 : 1.0);
    decayValue.setText (juce::String (juce::roundToInt (decaySeconds * 1000.0)) + " ms", juce::dontSendNotification);

    const auto toneText = toneSlider.getValue() < 0.35 ? "Dark" : toneSlider.getValue() < 0.7 ? "Bright" : "Cutting";
    const auto airText = airSlider.getValue() < 0.35 ? "Dry" : airSlider.getValue() < 0.7 ? "Sharp" : "Sprayed";
    const auto metalText = metalSlider.getValue() < 0.2 ? "Noise" : metalSlider.getValue() < 0.55 ? "Hybrid" : "Metallic";
    const auto driveText = driveSlider.getValue() < 0.3 ? "Clean" : driveSlider.getValue() < 0.65 ? "Crunch" : "Ripped";

    toneValue.setText (toneText, juce::dontSendNotification);
    airValue.setText (airText, juce::dontSendNotification);
    metalValue.setText (metalText, juce::dontSendNotification);
    driveValue.setText (driveText, juce::dontSendNotification);
    repaint();
}

void HeadturnerHatAudioProcessorEditor::timerCallback()
{
    updateModeButtons();
}

void HeadturnerHatAudioProcessorEditor::updateModeButtons()
{
    if (auto* openParam = audioProcessor.getValueTreeState().getRawParameterValue ("open"))
    {
        const auto isOpen = openParam->load() >= 0.5f;
        openButton.setToggleState (isOpen, juce::dontSendNotification);
        closedButton.setToggleState (! isOpen, juce::dontSendNotification);
    }
}

juce::File HeadturnerHatAudioProcessorEditor::getPresetDirectory() const
{
    auto directory = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Headturner Hat Presets");
    directory.createDirectory();
    return directory;
}

juce::String HeadturnerHatAudioProcessorEditor::sanitisePresetName (juce::String name) const
{
    name = name.trim();

    if (name.isEmpty())
        name = "Untitled Preset";

    return name.retainCharacters ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 -_");
}

void HeadturnerHatAudioProcessorEditor::saveCurrentPreset()
{
    const auto presetName = sanitisePresetName (presetNameEditor.getText());
    presetNameEditor.setText (presetName, juce::dontSendNotification);

    juce::MemoryBlock state;
    audioProcessor.getStateInformation (state);

    const auto presetFile = getPresetDirectory().getChildFile (presetName + ".headturnerhatpreset");
    presetFile.replaceWithData (state.getData(), state.getSize());
}

void HeadturnerHatAudioProcessorEditor::loadPreset()
{
    juce::PopupMenu menu;
    const auto presetDirectory = getPresetDirectory();
    const auto files = presetDirectory.findChildFiles (juce::File::findFiles, false, "*.headturnerhatpreset");

    if (files.isEmpty())
    {
        menu.addItem (1, "No saved presets", false, false);
    }
    else
    {
        int itemId = 1;

        for (const auto& file : files)
            menu.addItem (itemId++, file.getFileNameWithoutExtension());
    }

    menu.showMenuAsync (juce::PopupMenu::Options(),
                        [this, files] (int result)
                        {
                            if (result <= 0 || result > files.size())
                                return;

                            juce::MemoryBlock state;

                            if (files[result - 1].loadFileAsData (state))
                            {
                                audioProcessor.setStateInformation (state.getData(), static_cast<int> (state.getSize()));
                                presetNameEditor.setText (files[result - 1].getFileNameWithoutExtension(), juce::dontSendNotification);
                                refreshValueLabels();
                                updateModeButtons();
                            }
                        });
}

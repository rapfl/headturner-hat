#pragma once

#include <JuceHeader.h>

class HeadturnerHatAudioProcessor final : public juce::AudioProcessor
{
public:
    HeadturnerHatAudioProcessor();
    ~HeadturnerHatAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #if ! JucePlugin_IsMidiEffect
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS& getValueTreeState() noexcept { return parameters; }

    static APVTS::ParameterLayout createParameterLayout();

private:
    struct HatSound final : public juce::SynthesiserSound
    {
        bool appliesToNote (int) override { return true; }
        bool appliesToChannel (int) override { return true; }
    };

    struct VoiceParameters
    {
        float mode = 0.0f;
        float decay = 0.34f;
        float tone = 0.68f;
        float air = 0.62f;
        float metal = 0.22f;
        float drive = 0.41f;
    };

    class HatVoice final : public juce::SynthesiserVoice
    {
    public:
        HatVoice() = default;

        bool canPlaySound (juce::SynthesiserSound* sound) override;
        void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
        void stopNote (float velocity, bool allowTailOff) override;
        void pitchWheelMoved (int newPitchWheelValue) override;
        void controllerMoved (int controllerNumber, int newControllerValue) override;
        void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;

        void prepare (double sampleRate, int samplesPerBlock, int numChannels);
        void updateParameters (const VoiceParameters& newParams);

    private:
        static constexpr int numMetalOscillators = 6;

        struct NoiseGenerator
        {
            float current = 0.0f;
            juce::Random random;

            float processSample() noexcept
            {
                current = 0.82f * current + 0.18f * random.nextFloat() * 2.0f - 1.0f;
                return current;
            }
        };

        float getNextSample() noexcept;
        void updateFilters();
        void clearVoice() noexcept;

        double currentSampleRate = 44100.0;
        bool active = false;
        float envelope = 0.0f;
        float envelopeMultiplier = 0.999f;
        float velocityGain = 1.0f;
        float transientEnvelope = 0.0f;
        float transientMultiplier = 0.92f;
        float driveAmount = 1.0f;
        float previousNoiseSample = 0.0f;
        float phase[numMetalOscillators] {};
        float phaseDelta[numMetalOscillators] {};
        VoiceParameters params;

        NoiseGenerator noise;
        juce::dsp::StateVariableTPTFilter<float> highpass;
        juce::dsp::StateVariableTPTFilter<float> lowpass;
    };

    VoiceParameters readParameters() const;

    APVTS parameters;
    juce::Synthesiser synth;
    VoiceParameters cachedParams;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeadturnerHatAudioProcessor)
};

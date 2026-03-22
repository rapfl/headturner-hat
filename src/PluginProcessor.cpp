#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace IDs
{
    static constexpr auto open = "open";
    static constexpr auto decay = "decay";
    static constexpr auto tone = "tone";
    static constexpr auto air = "air";
    static constexpr auto metal = "metal";
    static constexpr auto drive = "drive";
}

HeadturnerHatAudioProcessor::HeadturnerHatAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    synth.clearVoices();
    synth.addVoice (new HatVoice());
    synth.clearSounds();
    synth.addSound (new HatSound());
    synth.setNoteStealingEnabled (true);
}

void HeadturnerHatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (getTotalNumOutputChannels()) };

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<HatVoice*> (synth.getVoice (i)))
            voice->prepare (spec.sampleRate, samplesPerBlock, static_cast<int> (spec.numChannels));

    cachedParams = readParameters();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<HatVoice*> (synth.getVoice (i)))
            voice->updateParameters (cachedParams);
}

void HeadturnerHatAudioProcessor::releaseResources()
{
}

#if ! JucePlugin_IsMidiEffect
bool HeadturnerHatAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void HeadturnerHatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const auto latestParams = readParameters();
    const bool changed = std::memcmp (&latestParams, &cachedParams, sizeof (VoiceParameters)) != 0;

    if (changed)
    {
        cachedParams = latestParams;

        for (int i = 0; i < synth.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<HatVoice*> (synth.getVoice (i)))
                voice->updateParameters (cachedParams);
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* HeadturnerHatAudioProcessor::createEditor()
{
    return new HeadturnerHatAudioProcessorEditor (*this);
}

bool HeadturnerHatAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String HeadturnerHatAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HeadturnerHatAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HeadturnerHatAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HeadturnerHatAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HeadturnerHatAudioProcessor::getTailLengthSeconds() const
{
    return 1.2;
}

int HeadturnerHatAudioProcessor::getNumPrograms()
{
    return 1;
}

int HeadturnerHatAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HeadturnerHatAudioProcessor::setCurrentProgram (int)
{
}

const juce::String HeadturnerHatAudioProcessor::getProgramName (int)
{
    return {};
}

void HeadturnerHatAudioProcessor::changeProgramName (int, const juce::String&)
{
}

void HeadturnerHatAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (const auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void HeadturnerHatAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (const auto xmlState = getXmlFromBinary (data, sizeInBytes))
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

HeadturnerHatAudioProcessor::APVTS::ParameterLayout HeadturnerHatAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterBool> (IDs::open, "Open", false));

    auto normalized = juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f);

    params.push_back (std::make_unique<juce::AudioParameterFloat> (IDs::decay, "Decay", normalized, 0.34f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (IDs::tone, "Tone", normalized, 0.68f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (IDs::air, "Air", normalized, 0.62f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (IDs::metal, "Metal", normalized, 0.22f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (IDs::drive, "Drive", normalized, 0.41f));

    return { params.begin(), params.end() };
}

HeadturnerHatAudioProcessor::VoiceParameters HeadturnerHatAudioProcessor::readParameters() const
{
    VoiceParameters result;
    result.mode = parameters.getRawParameterValue (IDs::open)->load();
    result.decay = parameters.getRawParameterValue (IDs::decay)->load();
    result.tone = parameters.getRawParameterValue (IDs::tone)->load();
    result.air = parameters.getRawParameterValue (IDs::air)->load();
    result.metal = parameters.getRawParameterValue (IDs::metal)->load();
    result.drive = parameters.getRawParameterValue (IDs::drive)->load();
    return result;
}

bool HeadturnerHatAudioProcessor::HatVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<HatSound*> (sound) != nullptr;
}

void HeadturnerHatAudioProcessor::HatVoice::prepare (double sampleRate, int, int)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, 512, 1 };
    highpass.reset();
    lowpass.reset();
    highpass.prepare (spec);
    lowpass.prepare (spec);
    updateFilters();
}

void HeadturnerHatAudioProcessor::HatVoice::updateParameters (const VoiceParameters& newParams)
{
    params = newParams;
    updateFilters();
    driveAmount = 1.0f + params.drive * 9.0f;
}

void HeadturnerHatAudioProcessor::HatVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    juce::ignoreUnused (midiNoteNumber);

    static constexpr std::array<float, numMetalOscillators> baseFrequencies { 431.0f, 617.0f, 823.0f, 1179.0f, 1487.0f, 1913.0f };

    const auto decaySeconds = (0.04f + params.decay * 0.42f) * (params.mode > 0.5f ? 2.2f : 1.0f);
    envelope = 1.0f;
    envelopeMultiplier = std::exp (std::log (0.0001f) / (static_cast<float> (currentSampleRate) * juce::jmax (0.02f, decaySeconds)));
    transientEnvelope = 1.0f;
    transientMultiplier = 0.3f;
    previousNoiseSample = 0.0f;
    velocityGain = juce::jlimit (0.0f, 1.3f, 0.45f + velocity * 0.95f);
    active = true;

    for (int i = 0; i < numMetalOscillators; ++i)
    {
        phase[i] = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
        const auto detune = juce::jmap (params.drive, 0.0f, 1.0f, 0.985f, 1.03f);
        const auto wobble = 1.0f + (juce::Random::getSystemRandom().nextFloat() - 0.5f) * (detune - 1.0f);
        phaseDelta[i] = juce::MathConstants<float>::twoPi * baseFrequencies[static_cast<size_t> (i)] * wobble / static_cast<float> (currentSampleRate);
    }
}

void HeadturnerHatAudioProcessor::HatVoice::stopNote (float, bool allowTailOff)
{
    if (! allowTailOff)
        clearVoice();
}

void HeadturnerHatAudioProcessor::HatVoice::pitchWheelMoved (int)
{
}

void HeadturnerHatAudioProcessor::HatVoice::controllerMoved (int, int)
{
}

void HeadturnerHatAudioProcessor::HatVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! active)
        return;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto value = getNextSample();

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample (channel, startSample + sample, value);
    }
}

float HeadturnerHatAudioProcessor::HatVoice::getNextSample() noexcept
{
    if (! active)
        return 0.0f;

    float metallic = 0.0f;

    for (int i = 0; i < numMetalOscillators; ++i)
    {
        const auto square = std::sin (phase[i]) >= 0.0f ? 1.0f : -1.0f;
        const auto triangle = std::asin (std::sin (phase[i])) * (2.0f / juce::MathConstants<float>::pi);
        const auto voice = (i % 2 == 0 ? square : triangle) * (0.18f / (1.0f + i * 0.08f));
        metallic += voice;
        phase[i] += phaseDelta[i];

        if (phase[i] > juce::MathConstants<float>::twoPi)
            phase[i] -= juce::MathConstants<float>::twoPi;
    }

    const auto rawNoise = noise.processSample();
    const auto brightNoise = rawNoise - previousNoiseSample * 0.82f;
    previousNoiseSample = rawNoise;

    const auto metallicBlend = params.metal
        * juce::jmap (params.tone, 0.0f, 1.0f, 0.04f, 0.42f)
        * juce::jmap (params.air, 0.0f, 1.0f, 0.16f, 0.85f);
    const auto noiseBlend = juce::jmap (params.tone, 0.0f, 1.0f, 0.18f, 0.32f)
        + juce::jmap (params.air, 0.0f, 1.0f, 0.08f, 0.28f)
        + juce::jmap (params.metal, 0.0f, 1.0f, 0.12f, -0.03f)
        + (params.mode > 0.5f ? 0.06f : 0.0f);
    const auto glassBlend = juce::jmap (params.tone, 0.0f, 1.0f, 0.14f, 0.38f)
        + juce::jmap (params.air, 0.0f, 1.0f, 0.06f, 0.42f)
        + juce::jmap (params.metal, 0.0f, 1.0f, 0.08f, -0.04f);

    auto sample = metallic * metallicBlend + rawNoise * noiseBlend + brightNoise * glassBlend;
    sample = highpass.processSample (0, sample);
    sample = lowpass.processSample (0, sample);

    const auto transient = (brightNoise * 0.8f + rawNoise * 0.2f) * transientEnvelope * 0.48f;
    transientEnvelope *= transientMultiplier;
    sample += transient;

    sample = std::tanh (sample * driveAmount) * envelope * velocityGain * 0.72f;
    envelope *= envelopeMultiplier;

    if (envelope < 0.00012f)
        clearVoice();

    return sample;
}

void HeadturnerHatAudioProcessor::HatVoice::updateFilters()
{
    const auto hpFrequency = juce::jmap (params.air, 0.0f, 1.0f, 6200.0f, 11800.0f);
    const auto lpFrequency = juce::jmap (params.tone, 0.0f, 1.0f, 9200.0f, 19500.0f);

    highpass.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lowpass.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    highpass.setCutoffFrequency (hpFrequency);
    lowpass.setCutoffFrequency (lpFrequency);
    highpass.setResonance (0.58f);
    lowpass.setResonance (0.22f);
}

void HeadturnerHatAudioProcessor::HatVoice::clearVoice() noexcept
{
    active = false;
    envelope = 0.0f;
    transientEnvelope = 0.0f;
    previousNoiseSample = 0.0f;
    clearCurrentNote();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HeadturnerHatAudioProcessor();
}

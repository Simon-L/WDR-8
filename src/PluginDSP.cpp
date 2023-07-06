/*
 * ImGui plugin example
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */
#include "PluginDSP.hpp"

START_NAMESPACE_DISTRHO

PluginDSP::PluginDSP()
    : Plugin(kParamCount, 0, 0) // parameters, programs, states
{
    for (int i = 0; i < kParamCount; ++i)
    {
        d_stdout("DSP %d -> %s (%s)", i, params.properties[i].name.buffer(), params.properties[i].symbol.buffer());
    }
}

void PluginDSP::activate()
{
    // fSmoothGain->flush();
    synth.prepare(getSampleRate());
    hat1.prepare(getSampleRate());

    synth.filter.OutputLPF.setCutoffFrequency(40000.0);
    synth.filter.OutputLPF.setQValue(1.0);

    for (int par = 0; par < kParamCount; ++par)
    {   
        setParameterValue(par, params.values[par]);
    }
}

void PluginDSP::run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount)
{

    for (int m = 0; m < midiEventCount; ++m)
    {   
        handleMidi(&midiEvents[m]);
    }

    // get the left and right audio outputs
    float* const outL = outputs[0];
    float* const outR = outputs[1];

    // apply gain against all samples
    for (uint32_t i=0; i < frames; ++i)
    {   
        hat1.process();
        outL[i] = (hat1.oh_out) * 0.5 * params.values[kVolume];
        outR[i] = (hat1.oh_out) * 0.5 * params.values[kVolume];

        // plot[plotIndex] = synth.freq;
        // plotIndex++;
        // if (plotIndex >= plotSize) {
        //     plotIndex = 0;
        // }
    }
}

void PluginDSP::sampleRateChanged(double newSampleRate)
{
    fSampleRate = newSampleRate;
    // fSmoothGain->setSampleRate(newSampleRate);
}

int PluginDSP::publicMethod()
{
    return 42;
}

// --------------------------------------------------------------------------------------------------------------------

Plugin* createPlugin()
{
    return new PluginDSP();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO

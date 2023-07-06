#include "PluginDSP.hpp"

void PluginDSP::initParameter(uint32_t index, Parameter& parameter)
{
    parameter.ranges.min = params.properties[index].ranges.min;
    parameter.ranges.max = params.properties[index].ranges.max;
    parameter.ranges.def = params.properties[index].ranges.def;
    parameter.hints = params.properties[index].hints;
    parameter.name = params.properties[index].name;
    parameter.shortName = params.properties[index].shortName;
    parameter.symbol = params.properties[index].symbol;
    parameter.unit = params.properties[index].unit;
    return;
}

float PluginDSP::getParameterValue(uint32_t index) const
{
    if (index < kParamCount)
        return params.values[index];
    return 0.0;
}

void PluginDSP::setParameterValue(uint32_t index, float value)
{
    d_stdout("DSP %d %s -> %f", index, params.properties[index].name.buffer(), value);
    params.values[index] = value;
    switch (index) {
    case kCutoff:
        synth.setCutoff(paramLog(params.values[kCutoff], cutoffLogBpX, cutoffLogBpY, cutoffLogMin, cutoffLogMax));
        d_stdout("\tDSP CUT OFF FREQ -> %f", paramLog(params.values[kCutoff], cutoffLogBpX, cutoffLogBpY, cutoffLogMin, cutoffLogMax));
        break;
    case kResonance:
        synth.setResonance(params.values[kResonance]);
        break;
    case kEnvMod:
        synth.setEnvmod(paramLog(params.values[kEnvMod], envModLogBpX, envModLogBpY, envModLogMin, envModLogMax));
        d_stdout("\tDSP ENV MOD FREQ -> %f", paramLog(params.values[kEnvMod], envModLogBpX, envModLogBpY, envModLogMin, envModLogMax));
        break;
    case kAccent:
        synth.setAccent(params.values[kAccent]);
        break;
    case kDecay:
        synth.setDecay(params.values[kDecay]);
        break;
    case kWaveform:
        value > 0.5 ? synth.useSquareWaveform() : synth.useSawWaveform();
        break;
    case kVcaDec:
        synth.setVcaDecay(params.values[kVcaDec]);
        break;
    case kMetalF1:
        hat1.metal.sq1.setFrequency(value);
        break;
    case kMetalF2:
        hat1.metal.sq2.setFrequency(value);
        break;
    case kMetalF3:
        hat1.metal.sq3.setFrequency(value);
        break;
    case kMetalF4:
        hat1.metal.sq4.setFrequency(value);
        break;
    case kMetalF5:
        hat1.metal.sq5.setFrequency(value);
        break;
    case kMetalF6:
        hat1.metal.sq6.setFrequency(value);
        break;
    case kCHhpfFreq:
        hat1.setChHpfFreq(value);
        break;
    case kCHhpfQ:
        hat1.setChHpfQ(value);
        break;
    case kOHhpfFreq:
        hat1.setOhHpfFreq(value);
        break;
    case kOHhpfQ:
        hat1.setOhHpfQ(value);
        break;
    case kCHdec:
        hat1.setChDec(value);
        break;
    case kOHdec:
        hat1.setOhDec(value);
        break;
    case kOHdec2:
        hat1.setOhDec2(value);
        break;
    }
}
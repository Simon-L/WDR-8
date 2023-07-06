#include "chowdsp_sources/chowdsp_sources.h"
#include "chowdsp_dsp_utils/chowdsp_dsp_utils.h"
#include "chowdsp_filters/chowdsp_filters.h"

struct OscMetal {

    float pow = 0.0f;
    chowdsp::SquareWave<double> sq1;
    chowdsp::SquareWave<double> sq2;
    chowdsp::SquareWave<double> sq3;
    chowdsp::SquareWave<double> sq4;
    chowdsp::SquareWave<double> sq5;
    chowdsp::SquareWave<double> sq6;
    juce::dsp::ProcessSpec spec;

    float cv;

    // set CV value, accepted range is 0v-5.0v
    void setPitchCV(float value) {
        cv = value;
        // sq1.setFrequency(142);
        // sq2.setFrequency(211);
        // sq3.setFrequency(297);
        // sq4.setFrequency(385);
        // sq5.setFrequency(800);
        // sq6.setFrequency(540);
        // 205.3, 369.6, 304.4, 522.7, 800, 540
        sq1.setFrequency(205.347);
        sq2.setFrequency(369.624);
        sq3.setFrequency(304.396);
        sq4.setFrequency(522.701);
        sq5.setFrequency(800);
        sq6.setFrequency(540.54);
    }

    void process(double* buf, uint32_t frames) {
        float f;
        for (uint32_t i=0; i < frames; ++i)
        {
            double out =
                sq1.processSample() * 0.04 +
                sq2.processSample() * 0.04 +
                sq3.processSample() * 0.04 +
                sq4.processSample() * 0.04 +
                sq5.processSample() * 0.04 +
                sq6.processSample() * 0.04;
            buf[i] = out;
        }
    }

    void prepare(float sampleRate, float defaultCV = 1.0) {
        spec.maximumBlockSize = 2048;
        spec.numChannels = 1;
        spec.sampleRate = sampleRate;
        sq1.prepare(spec);
        sq2.prepare(spec);
        sq3.prepare(spec);
        sq4.prepare(spec);
        sq5.prepare(spec);
        sq6.prepare(spec);
        setPitchCV(defaultCV);
    }

};
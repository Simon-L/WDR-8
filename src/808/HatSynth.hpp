#include "chowdsp_dsp_utils/chowdsp_dsp_utils.h"

// #include "../303/ADAREnvelope.h"
// #include "../303/WowFilter.h"
// #include "../303/Osc303.hpp"
// #include "../303/AcidFilter.hpp"
// #include "../303/SlideFilter.hpp"
// #include "../303/AcidFormulas.hpp"

#include "Resonator.hpp"
#include "OscMetal.hpp"
#include "digital.hpp"

struct HatSynth
{   
    static constexpr int BLOCK_SIZE{32};
    float sawBuffer[4];
    Osc303 osc;
    AcidFilter filter;
    WowFilter wowFilter;
    SlideFilter slideFilter;
    
    HatResonatorWDF reso;
    OscMetal metal;
    float metalBuffer[4];
    
    chowdsp::ButterworthFilter< 2, chowdsp::ButterworthFilterType::Highpass, float> fi;

    float vcfDecTime = 1.223;
    float vcaDecTime = 2.0;

    float freq = 0.0;

    float Vcutoff = 3.398;
    float Resonance = 1.0;
    float Envmod = 0.887;
    float Accent = 1.0;

    float A = 2.22;
    float B = 0.31;
    float C = 0.434;
    float D = 0.372;
    float E = 4.549;
    float VaccMul = 2.566;
    inline void setFormulaA(float value) { A = value; }
    inline void setFormulaB(float value) { B = value; }
    inline void setFormulaC(float value) { C = value; }
    inline void setFormulaD(float value) { D = value; }
    inline void setFormulaE(float value) { E = value; }
    inline void setFormulaVaccMul(float value) { VaccMul = value; }

    sst::surgext_rack::dsp::envelopes::ADAREnvelope vca_env;
    sst::surgext_rack::dsp::envelopes::ADAREnvelope vcf_env;
    
    rack::dsp::PulseGenerator trigger;

    float OutputLPFFreq = 45630.0;
    float OutputLPResonance = 0.27;

    float sampleRate;
    float sampleTime;

    float note_cv;
    uint8_t wfm = 1; // 1 == saw, 0 == square
    void useSawWaveform() { wfm = 1; d_stdout("Using wfm SAW"); }
    void useSquareWaveform() { wfm = 0; d_stdout("Using wfm PULSE"); }

    inline void setCutoff(float value) { Vcutoff = value; }
    inline void setResonance(float value) { Resonance = value; }
    inline void setEnvmod(float value) { Envmod = value; }
    inline void setAccent(float value) { Accent = value; }
    inline void setDecay(float value) { vcfDecTime = value; }
    inline void setVcaDecay(float value) { vcaDecTime = value; }

    bool gate = false;
    bool accent = false;
    bool slide = false;

    inline void setMidiNote(uint8_t m) { note_cv = (std::clamp((int)m, 12, 72) - 12) / 12.0; }
    inline void slideMidiNote(uint8_t m) { slide = true; note_cv = (std::clamp((int)m, 12, 72) - 12) / 12.0; }
    
    float Atime = log2(1.5e-3f);
    float H = log2(2.2e-3f);
    float Rtime = log2(100e-3f);

    void gateOn(bool accent) {
        gate = true;
        slide = false;
        this->accent = accent;
        trigger.trigger(3.7e-3f);
        vca_env.attackFrom(0.0, 1, false, true); // from, shape, isDigital, isGated
    }

    void gateOff() {
        gate = false;
    }

    void panic() {
        gate = false;
    }

    void prepare(float sampleRate) {
        this->sampleRate = sampleRate;
        this->sampleTime = 1.0/sampleRate;
        
        reso.prepare(sampleRate);
        reso.setParameters(82e3, 560.0, 3.3e-9);
        
        metal.prepare(sampleRate);
        metal.setPitchCV(1.0f);
        
        fi.prepare(1);
        fi.calcCoefs(12240, 1.0, sampleRate);

        vca_env.activate(sampleRate);
        vcf_env.activate(sampleRate);
        wowFilter.prepare(sampleRate);
        slideFilter.prepare(sampleRate);

        filter.prepare(sampleRate, 300.0, 0.66);
    }
    
    float process() {
        metal.process(metalBuffer, 1);

        float reso_out = reso.processSample(metalBuffer[0]);
        
        auto b = trigger.process(sampleTime);

        vca_env.process(Atime, Rtime, 1, 1, b); // atk, dec, atk shape, dec shape, gate

        float amp = vca_env.output;
        float hpf_out = fi.processSample(reso_out);
        return hpf_out * amp;
    }
};

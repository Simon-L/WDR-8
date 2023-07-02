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

#include "sst/basic-blocks/modulators/ADAREnvelope.h"
#include "sst/basic-blocks/modulators/ADSREnvelope.h"

struct HatSynth
{   
    static constexpr int BLOCKSIZE{32};
    float sawBuffer[4];
    Osc303 osc;
    AcidFilter filter;
    WowFilter wowFilter;
    SlideFilter slideFilter;
    
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
    
    // sst::surgextOhRtimeack::dsp::envelopes::ADAREnvelope vcf_env;
    
    float OutputLPFFreq = 45630.0;
    float OutputLPResonance = 0.27;
    // sst::surgextOhRtimeack::dsp::envelopes::ADAREnvelope vca_env;
    float note_cv;
    uint8_t wfm = 1; // 1 == saw, 0 == square
    // void useSawWaveform() { wfm = 1; dOhSustaintdout("Using wfm SAW"); }
    // void useSquareWaveform() { wfm = 0; dOhSustaintdout("Using wfm PULSE"); }
    
    inline void setCutoff(float value) { Vcutoff = value; }
    inline void setResonance(float value) { Resonance = value; }
    inline void setEnvmod(float value) { Envmod = value; }
    inline void setAccent(float value) { Accent = value; }
    inline void setDecay(float value) { vcfDecTime = value; }
    inline void setVcaDecay(float value) { vcaDecTime = value; }
    inline void setMidiNote(uint8_t m) { note_cv = (std::clamp((int)m, 12, 72) - 12) / 12.0; }
    inline void slideMidiNote(uint8_t m) { slide = true; note_cv = (std::clamp((int)m, 12, 72) - 12) / 12.0; }
    
    
    HatResonatorWDF reso;
    OscMetal metal;
    double metalBuffer[4];
    chowdsp::ButterworthFilter< 2, chowdsp::ButterworthFilterType::Highpass, float> ch_hpf;
    chowdsp::ButterworthFilter< 2, chowdsp::ButterworthFilterType::Highpass, float> oh_hpf;
    
    rack::dsp::PulseGenerator ChTrigger;
    rack::dsp::PulseGenerator OhTrigger;

    float sampleRate;
    float sampleTime;

    struct SampleSRProvider
    {
        double samplerate, sampleRateInv;
        void prepare(double sr) { samplerate = sr; sampleRateInv = 1.f / samplerate; }
        float envelope_rate_linear_nowrap(float f) const { return 16 * sampleRateInv * pow(2.f, -f); }
    } srp;

    std::unique_ptr<sst::basic_blocks::modulators::ADAREnvelope<SampleSRProvider, 16>> ch_env;
    std::unique_ptr<sst::basic_blocks::modulators::ADSREnvelope<SampleSRProvider, 16>> oh_env;
    
    bool gate = false;
    bool accent = false;
    bool slide = false;
    
    float ChAtime = log2(0.5e-3f);
    float H = log2(2.2e-3f);
    float ChRtime = log2(70e-3f);
    
    float OhAtime = 0.001;
    float OhDtime = 0.75;
    float OhSustain = 0.0f;
    float OhRtime = 4 * 0.10;
    
    float OhHoldtime = 120e-3f;
    
    float ch_out = 0.0;
    float oh_out = 0.0;
    
    void ChGateOn(bool accent) {
        printf("Go Ch!\n");
        ChTrigger.trigger(3.7e-3f);
        ch_env.get()->attackFrom(0.0, 1, false, true); // from, shape, isDigital, isGated
    }

    void OhGateOn(bool accent) {
        printf("Go Oh!\n");
        OhTrigger.trigger(OhHoldtime);
        oh_env.get()->attackFrom(0.0, OhAtime, 1, false); // initial, attacktime, ashp, digital?
    }
    
    void gateOff() {
        // gate = false;
    }

    void panic() {
        // gate = false;
    }
    
    void setOhDec(float decay) {
        OhHoldtime = decay;
    }
    void setChDec(float decay) {
        ChRtime = log2(decay);
    }
    
    void setChHpf(float freq) {
        ch_hpf.calcCoefs(freq, 1.0, sampleRate);
    }

    void prepare(float sampleRate) {
        this->sampleRate = sampleRate;
        this->sampleTime = 1.0/sampleRate;
        
        srp.prepare(sampleRate);
        
        ch_env = std::make_unique<sst::basic_blocks::modulators::ADAREnvelope<SampleSRProvider, 16>>(&srp);
        oh_env = std::make_unique<sst::basic_blocks::modulators::ADSREnvelope<SampleSRProvider, 16>>(&srp);
        
        reso.prepare(sampleRate);
        
        metal.prepare(sampleRate);
        metal.setPitchCV(1.0f);
        
        ch_hpf.prepare(1);
        ch_hpf.calcCoefs(12200, 1.0, sampleRate);
        oh_hpf.prepare(1);
        oh_hpf.calcCoefs(8200, 1.0, sampleRate);
    }
    
    void process() {
        metal.process(metalBuffer, 1);
        float reso_out = reso.processSample(metalBuffer[0]);

        auto ChGate = ChTrigger.process(sampleTime);
        ch_env.get()->processScaledAD(ChAtime, ChRtime, 1, 1, ChGate); // atk, dec, atk shape, dec shape, gate
        float ChAmp = ch_env.get()->output;
        float ch_hpf_out = ch_hpf.processSample(reso_out);
        ch_out = ch_hpf_out * ChAmp;

        auto OhGate = OhTrigger.process(sampleTime);
        oh_env.get()->process(OhAtime, OhDtime, OhSustain, OhRtime, 1, 1, 1, OhGate); // a, d, s, r, ashp, dshp, rshp, gateActive
        float OhAmp = oh_env.get()->output;
        float oh_hpf_out = oh_hpf.processSample(reso_out);
        oh_out = oh_hpf_out * OhAmp;
    }
};

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
    chowdsp::ButterworthFilter<2, chowdsp::ButterworthFilterType::Highpass, float> ch_hpf;
    chowdsp::ButterworthFilter<2, chowdsp::ButterworthFilterType::Highpass, float> oh_hpf;
    
    chowdsp::FirstOrderHPF<float> chOutHpf;
    chowdsp::FirstOrderLPF<float> chOutLpf;
    chowdsp::FirstOrderHPF<float> ohOutHpf;
    chowdsp::FirstOrderLPF<float> ohOutLpf;
    
    rack::dsp::PulseGenerator ChTrigger;
    rack::dsp::PulseGenerator OhTrigger;

    float sampleRate;
    float sampleTime;
    
    struct EnvTime
    {
        static constexpr float defaultEtMin{-8}, defaultEtMax{3.32192809489}; // log2(10)
        float etMin{defaultEtMin}, etMax{defaultEtMax};
        
        EnvTime() {}
        EnvTime(float etMin, float etMax)
            : etMin(etMin), etMax(etMax) {}
        
        float value{etMin};

        float getValue() { return value; }
        void setValue(float nvalue) { value = nvalue; }
        
        std::string getDisplayValueString()
        {
            auto v = getValue() * (etMax - etMin) + etMin;

            if (getValue() < 0.0001)
            {
                std::string mv;
                if (getMinString(mv))
                {
                    return mv;
                }
            }
            char valstring[32];
            sprintf(valstring, "%.4f s", pow(2, v));
            return std::string(valstring);
        }
        
        void printDisplayValueString(float value)
        {
            auto v = value * (etMax - etMin) + etMin;
            char valstring[32];
            sprintf(valstring, "%.4f s", pow(2, v));
            printf("Value %f -> Time %s\n", value, valstring);

        }
        void setDisplayValueString(std::string s)
        {
            auto q = std::atof(s.c_str());
            auto v = log2(std::clamp(q, pow(2., etMin), pow(2., etMax)));
            auto vn = (v - etMin) / (etMax - etMin);
            setValue(vn);
        }
        
        float getValueTime(float q)
        {
            auto v = log2(std::clamp(double(q), pow(2., etMin), pow(2., etMax)));
            auto vn = (v - etMin) / (etMax - etMin);
            return vn;
        }

        virtual bool getMinString(std::string &s) { return false; }
    };

    struct SRProvider
    {
        double samplerate, sampleRateInv;
        void prepare(double sr) { samplerate = sr; sampleRateInv = 1.f / samplerate; }
        float envelope_rate_linear_nowrap(float f) const { return 8 * sampleRateInv * pow(2.f, -f); }
    } srp;
    
    struct ShortRange
    {
        // 100µs -> 4s
        static constexpr float etMin{-13.287}, etMax{2.0};
    };
    
    EnvTime et{ShortRange::etMin, ShortRange::etMax};
    
    std::unique_ptr<sst::basic_blocks::modulators::ADSREnvelope<SRProvider, 8, ShortRange>> ch_env;
    std::unique_ptr<sst::basic_blocks::modulators::ADSREnvelope<SRProvider, 8, ShortRange>> oh_env;
    
    bool gate = false;
    bool accent = false;
    bool slide = false;
    
    // closed etMin{-13.287}, etMax{2.0}; 	"tEnvA": 0.5e-3, D = 0, S = 1.0, "tEnvR": 60e-3, trigger = 1e-3
    // open etMin{-13.287}, etMax{2.0}; 	"tEnvA": 0.5e-3, D = 1.25s, S = 0, "tEnvR": 100e-3, trigger = short 40e-3 long 700e-3
    
    float ChAtime = et.getValueTime(0.5e-3);
    float ChDtime = et.getValueTime(0.0);
    float ChSustain = 1.0;
    float ChRtime = et.getValueTime(60e-3);
    
    float OhAtime = et.getValueTime(0.5e-3);
    float OhDtime = et.getValueTime(1.25);
    float OhSustain = 0.0;
    float OhRtime = et.getValueTime(100e-3);
    
    float OhHoldtime = 40e-3f;
    
    float ch_out = 0.0;
    float oh_out = 0.0;
    
    void ChGateOn(bool accent) {
        printf("Go Ch!\n");
        ChTrigger.trigger(1e-3f);
        ch_env.get()->attackFrom(0.0, ChAtime, 1, false); // initial, attacktime, ashp, digital?
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
    void setOhDec2(float decay) {
        // OhDtime = decay;
    }
    void setChDec(float decay) {
        // ChRtime = log2(decay);
    }
    
    float chHpfQ, chHpfFreq;
    void setChHpfQ(float q) {
        // chHpfQ = q;
        // ch_hpf.calcCoefs(chHpfFreq, chHpfQ, sampleRate);
    }
    
    void setChHpfFreq(float freq) {
        // chHpfFreq = freq;
        // ch_hpf.calcCoefs(chHpfFreq, chHpfQ, sampleRate);
    }
    
    float ohHpfQ, ohHpfFreq;
    void setOhHpfQ(float q) {
        // ohHpfQ = q;
        // oh_hpf.calcCoefs(ohHpfFreq, ohHpfQ, sampleRate);
    }
    
    void setOhHpfFreq(float freq) {
        // ohHpfFreq = freq;
        // oh_hpf.calcCoefs(ohHpfFreq, ohHpfQ, sampleRate);
    }

    void prepare(float sampleRate) {
        this->sampleRate = sampleRate;
        this->sampleTime = 1.0/sampleRate;
        
        srp.prepare(sampleRate);
        
        ch_env = std::make_unique<sst::basic_blocks::modulators::ADSREnvelope<SRProvider, 8, ShortRange>>(&srp);
        oh_env = std::make_unique<sst::basic_blocks::modulators::ADSREnvelope<SRProvider, 8, ShortRange>>(&srp);
        
        reso.prepare(sampleRate);
        reso.setParameters(82e3, 425, 3.79e-9);
        
        metal.prepare(sampleRate);
        metal.setPitchCV(1.0f);
        
        // values oh ButterworthFilter 2 freq 7750 q 1.95
        // values ch ButterworthFilter 2 freq 11450 q 1.55
        // values ch output gain 2 FirstOrderHPF 8000 -> FirstOrderLPF 11000 ( -1500 <- 9500 -> +1500)
        // values oh output gain 1.3 FirstOrderHPF 9500 -> FirstOrderLPF 12500 ( -1500 <- 11000 -> +1500)
        // reso Resonator rfb 82e3 r_g 425 c 3.79e-9 gain 0.325
        
        chHpfFreq = 11450;
        chHpfQ = 1.55;
        ch_hpf.prepare(1);
        ch_hpf.calcCoefs(chHpfFreq, chHpfQ, sampleRate);
        
        ohHpfFreq = 7750;
        ohHpfQ = 1.95;
        oh_hpf.prepare(1);
        oh_hpf.calcCoefs(ohHpfFreq, ohHpfQ, sampleRate);
        
        chOutHpf.reset();
        chOutHpf.prepare(1);
        chOutHpf.calcCoefs(8000, sampleRate);
        chOutLpf.reset();
        chOutLpf.prepare(1);
        chOutLpf.calcCoefs(11000, sampleRate);
        
        ohOutHpf.reset();
        ohOutHpf.prepare(1);
        ohOutHpf.calcCoefs(9500, sampleRate);
        ohOutLpf.reset();
        ohOutLpf.prepare(1);
        ohOutLpf.calcCoefs(12500, sampleRate);
    }
    
    void process() {
        metal.process(metalBuffer, 1);
        float reso_out = reso.processSample(metalBuffer[0]);

        auto ChGate = ChTrigger.process(sampleTime);
        ch_env.get()->process(ChAtime, ChDtime, ChSustain, ChRtime, 1, 1, 1, ChGate); // a, d, s, r, ashp, dshp, rshp, gateActive
        float ChAmp = ch_env.get()->output;
        float ch_hpf_out = ch_hpf.processSample(reso_out * 1.31);
        ch_hpf_out = chOutLpf.processSample(chOutHpf.processSample(ch_hpf_out));
        ch_out = ch_hpf_out * ChAmp;

        auto OhGate = OhTrigger.process(sampleTime);
        oh_env.get()->process(OhAtime, OhDtime, OhSustain, OhRtime, 1, 1, 1, OhGate); // a, d, s, r, ashp, dshp, rshp, gateActive
        float OhAmp = oh_env.get()->output;
        float oh_hpf_out = oh_hpf.processSample(reso_out * 1.15);
        oh_hpf_out = ohOutLpf.processSample(ohOutHpf.processSample(oh_hpf_out));
        oh_out = oh_hpf_out * OhAmp;
    }
};

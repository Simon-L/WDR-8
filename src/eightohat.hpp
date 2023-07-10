enum Parameters {
    kCutoff = 0,
    kResonance,
    kEnvMod,
    kDecay,
    kAccent,
    kHoldVca,
    kWaveform,
    kTuning,
    kVcaDec,
    kMetalF1,
    kMetalF2,
    kMetalF3,
    kMetalF4,
    kMetalF5,
    kMetalF6,
    kCHhpfFreq,
    kCHhpfQ,
    kOHhpfFreq,
    kOHhpfQ,
    kCHdec,
    kOHdec,
    kOHdec2,
    kVolume,
    kParamCount
};

inline float linear(float x, float lo_bp, float hi_bp, float lo_y, float hi_y) {
    float Xmapping = 1.0f/(hi_bp - lo_bp);
    float mappedX = Xmapping * (x - lo_bp);
    return (hi_y - lo_y) * mappedX + lo_y;
}

inline float paramLog(float x, float bpx, float bpy, float min, float max) {
    float bpy_mapped = (max - min) * bpy + min;
    if (x < bpx) return linear(x, 0.0f, bpx, min, bpy_mapped);
    else return linear(x, bpx, 1.0f, bpy_mapped, max);
}

static constexpr float cutoffLogBpX{0.5};
static constexpr float cutoffLogBpY{0.1};
static constexpr float cutoffLogMin{0.85};
static constexpr float cutoffLogMax{7.4};

static constexpr float envModLogBpX{0.5};
static constexpr float envModLogBpY{0.1};
static constexpr float envModLogMin{0.25};
static constexpr float envModLogMax{0.887};

struct eightohatParameters
{
    // Parameter(uint32_t h,const char * n,const char * s,const char * u,float def,float min,float max),
    Parameter properties[kParamCount] = {
        Parameter(kParameterIsAutomatable, "Cutoff frequency", "cutoff_freq", "", 0.5f, 0.0, 1.0), // Cutoff
        Parameter(kParameterIsAutomatable, "Resonance", "resonance", "", 0.5f, 0.0, 1.0), // Resonance
        Parameter(kParameterIsAutomatable, "Env Mod", "env_mod", "", 0.5f, 0.0, 1.0), // EnvMod
        Parameter(kParameterIsAutomatable, "Decay", "decay", "", -2.0f, -2.0, 1.223), // Decay
        Parameter(kParameterIsAutomatable, "Accent", "accent", "", 0.5f, 0.0, 1.0), // Accent
        Parameter(kParameterIsAutomatable|kParameterIsBoolean, "Hold VCA", "hold_vca", "", 0.0f, 0.0, 1.0), // HoldVca
        Parameter(kParameterIsAutomatable|kParameterIsBoolean, "Waveform", "waveform", "", 0.0f, 0.0, 1.0), // Waveform
        Parameter(kParameterIsAutomatable, "Tuning", "tuning", "", 0.0f, -1.0, 1.0), // Tuning
        Parameter(kParameterIsAutomatable, "VCA Dec", "vca_dec", "", 2.0f, -3.5, 6.0), // VcaDec
        Parameter(kParameterIsAutomatable, "metal f1", "metalf1", "", 205.3f, 100, 1600.0),
        Parameter(kParameterIsAutomatable, "metal f2", "metalf2", "", 369.6f, 100, 1600.0),
        Parameter(kParameterIsAutomatable, "metal f3", "metalf3", "", 304.3f, 100, 1600.0),
        Parameter(kParameterIsAutomatable, "metal f4", "metalf4", "", 522.7f, 100, 1600.0),
        Parameter(kParameterIsAutomatable, "metal f5", "metalf5", "", 800.0f, 100, 1600.0),
        Parameter(kParameterIsAutomatable, "metal f6", "metalf6", "", 540.54f, 100, 1600.0),
        Parameter(kParameterIsAutomatable, "CH hpf Freq", "chHpfFreq", "", 11550.0f, 7000.0f, 20000.0),
        Parameter(kParameterIsAutomatable, "CH hpf Q", "chHpfQ", "", 1.4f, 0.01f, 2.0),
        Parameter(kParameterIsAutomatable, "OH hpf Freq", "ohHpfFreq", "", 7580.0f, 7000.0f, 20000.0),
        Parameter(kParameterIsAutomatable, "OH hpf Q", "ohHpfQ", "", 1.28f, 0.01f, 2.0),
        Parameter(kParameterIsAutomatable, "CH dec", "ch_dec", "", 81e-3f, 20e-3f, 1500e-3f), // closed hat dec
        Parameter(kParameterIsAutomatable, "OH dec", "oh_dec", "", 40e-3f, 40e-3f, 700e-3f), // open hat dec
        Parameter(kParameterIsAutomatable, "OH dec2", "oh_dec2", "", 0.55, 0.001, 1.5), // open hat dec2
        Parameter(kParameterIsAutomatable, "Volume", "volume", "", 0.6303f, 0.0, 1.0), // Volume
    };

    float values[kParamCount];
    int v_waveform;

    eightohatParameters() {
        for (int i = 0; i < kParamCount; ++i)
        {
            values[i] = properties[i].ranges.def;
        }
    }

    float* get(uint index) {
        if (index >= kParamCount)
            return nullptr;
        return &values[index];
    }

    void set(uint index, float value) {
        if (index >= kParamCount)
            return;
        values[index] = value;
    }
};
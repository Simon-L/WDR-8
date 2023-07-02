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
    kCHhpf,
    kCHdec,
    kOHdec,
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
        Parameter(kParameterIsAutomatable, "CH hpf", "ch_hpf", "", 12200.0f, 9000.0f, 17000.0), // closed hat hpf
        Parameter(kParameterIsAutomatable, "CH dec", "ch_dec", "", 70e-3f, 20e-3f, 1500e-3f), // closed hat dec
        Parameter(kParameterIsAutomatable, "OH dec", "oh_dec", "", 120e-3f, 40e-3f, 650e-3f), // open hat dec
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
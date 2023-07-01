#include "PluginDSP.hpp"

#include <cstdint>

void PluginDSP::handleMidi(const MidiEvent* event)
{   
    uint8_t b0 = event->data[0]; // status + channel
    uint8_t b1 = event->data[1]; // note
    uint8_t b2 = event->data[2]; // velocity
    d_stdout("MIDI in 0x%x %d %d", b0, b1, b2);
    // if ((b0 != 0x90) || (b0 != 0x80)) {
    //     d_stdout("Blaaa");
    //     continue;
    // }
    if ((b0 == 0xb0) && (b1 == 123)) {
        d_stdout("All notes off");
        synth.panic();
        return;
    }
    if (b0 == 0x90) {
        if (b1 == 42) // GM note #42 F#1 : Closed Hi Hat
        {
            hat1.ChGateOn(accent);
        }
        
        if (b1 == 46) // GM note #46 Bb1 : Open Hi Hat
        {
            hat1.OhGateOn(accent);
        }
    }
}
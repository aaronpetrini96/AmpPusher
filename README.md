# AmpPusher

A machine-learning project, built while learning the basics: two neural overdrive units based on the classic green and yellow pedals we all love, as a JUCE plugin (AU, VST3).

## How it works

Each unit is a small GRU network (hidden size 32) conditioned on Drive and Tone.
It runs in real time through [RTNeural](https://github.com/jatinchowdhury18/RTNeural), with no per-sample allocation in the audio thread.

- The networks are trained offline in Python (PyTorch) from audio rendered through existing pedal emulations.
- Output level is a measured gain curve in C++, not learned.
- A C++ test checks that the plugin matches the PyTorch model for both pedals.

## Results

Null depth against the source emulation on held-out audio, across all 25 Drive × Tone settings (higher is better):

| Unit | Null depth |
|---|---|
| Green | 23.8 – 27.8 dB |
| Yellow | 26.7 – 30.9 dB |

## Tech

C++20, JUCE, RTNeural, CMake, Python (PyTorch).

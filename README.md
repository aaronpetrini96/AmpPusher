# AmpPusher

A machine-learning project, built while learning the basics: two neural overdrive units based on the classic green and yellow pedals we all love, as a JUCE plugin (AU, VST3).

Version 1.0.0.

## How it works

Each unit is a small GRU network (hidden size 32) conditioned on Drive and Tone.
It runs in real time through [RTNeural](https://github.com/jatinchowdhury18/RTNeural), with no per-sample allocation in the audio thread.

- The networks are trained offline in Python (PyTorch) from audio rendered through existing pedal emulations.
- Output level is a measured gain curve in C++, not learned.
- The networks were trained at 48 kHz and always run at 48 kHz. At other host rates the plugin resamples around them, so the sound does not change with the session's sample rate. The added latency (13 samples at 44.1 kHz, 24 at 96 kHz, none at 48 kHz) is reported to the host for compensation.
- Bypass crossfades to a latency-matched dry signal, and the networks stop running while bypassed.
- Switching between the two units crossfades, so it is click-free even when switched quickly.
- A C++ test checks that the plugin matches the PyTorch model for both pedals.

## Controls

Drive, Tone and Level knobs, a Mode switch for the green or yellow unit, and a bypass footswitch.
The lines icon in the corner sets the UI size (Small, Normal, Large, X-Large), which is remembered for new instances.

## Results

Null depth against the source emulation on held-out audio, across all 25 Drive × Tone settings, measured at 48 kHz (higher is better):

| Unit | Null depth |
|---|---|
| Green | 23.8 – 27.8 dB |
| Yellow | 26.7 – 30.9 dB |

## Tech

C++20, JUCE, RTNeural, CMake, Python (PyTorch).

## Formats

- macOS: AU and VST3

## Building

Requires CMake 3.22 or newer, Ninja and a C++20 compiler. JUCE and RTNeural are included as Git submodules.

```bash
git clone --recurse-submodules https://github.com/aaronpetrini96/AmpPusher.git
cd AmpPusher
```

macOS (universal binary):

```bash
cmake --preset mac_ninja_multi
cmake --build --preset mac_ninja_multi_release
```

The built plugins are copied to the system plugin folders automatically.

## Project structure

```
source/
  PluginProcessor.*   Parameters, signal chain and state
  PluginEditor.*      Editor layout and UI size menu
  DSP/                48 kHz resampler around the models
  UI/                 Components, popups and LookAndFeel classes
  Utils/              Parameter names, bypass, UI sizing and helpers
models/               Model weights loaded by the plugin
tests/                C++ parity test
Resources/            Images, icons and fonts
```

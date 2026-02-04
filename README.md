# Envelope Generator

A JUCE audio plugin with a 4-lane step sequencer that triggers AHD (Attack-Hold-Decay) envelopes to modulate filter cutoff or volume.

## Features

- **4 Independent Envelope Lanes**: Each lane has its own 16-step sequencer and AHD envelope
- **Flexible Routing**: Route each envelope to either filter cutoff or volume
- **State Variable Filter**: Choose between Lowpass, Highpass, or Bandpass modes
- **Resonance Control**: Adjustable filter resonance
- **DAW Tempo Sync**: Sequencer rates sync to host tempo (1/1 to 1/32)
- **Visual Feedback**: Step buttons show current playback position

## Building

### Prerequisites

- CMake 3.22 or higher
- C++17 compatible compiler
- Git (for the JUCE submodule)

### Build Steps

**First-time clone** (JUCE is included as a git submodule):

```bash
# Clone the repository and init the JUCE submodule
git clone --recursive <repo-url>
cd env_gen
```

If you already cloned without `--recursive`, run:

```bash
git submodule update --init --recursive
```

Then build:

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release
```

**After pulling:** if the submodule pointer changed, run `git submodule update --init --recursive` before configuring/building.

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### macOS (Xcode)

```bash
mkdir build
cd build
cmake .. -G Xcode
cmake --build . --config Release
```

## Plugin Formats

The plugin builds as:
- VST3
- Standalone application

## Usage

1. Load the plugin on an audio track in your DAW
2. Set the filter mode (Lowpass, Highpass, or Bandpass)
3. Adjust base cutoff frequency and resonance
4. For each lane:
   - Click step buttons to create a trigger pattern
   - Adjust Attack, Hold, and Decay times
   - Set the sequencer rate
   - Choose destination (Filter Cutoff or Volume)
5. Press play in your DAW to hear the envelopes trigger

## Project Structure

```
env_gen/
├── CMakeLists.txt              # CMake build configuration
├── JUCE/                       # JUCE framework (git submodule)
├── Source/
│   ├── PluginProcessor.h/cpp   # Audio processing and parameters
│   ├── PluginEditor.h/cpp      # Main UI
│   ├── DSP/
│   │   ├── Envelope.h/cpp      # AHD envelope generator
│   │   ├── Filter.h/cpp        # State variable filter
│   │   └── StepSequencer.h/cpp # Tempo-synced step sequencer
│   └── Components/
│       ├── CustomLookAndFeel.h/cpp  # UI styling
│       ├── StepButton.h/cpp         # Step button component
│       └── EnvelopeLane.h/cpp       # Lane UI component
└── README.md
```

## License

See LICENSE file for details.

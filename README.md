# NTS-1 Plugins

Custom oscillators and effects for the **Korg NTS-1 mk1** (nutekt-digital platform, logue SDK v1).

Each plugin compiles to a `.ntkdigunit` file, ready to load via the Korg NTS-1 Librarian.

---

## Oscillators

### MarkovWave ✅

> A Markov chain morphs between four waveforms (sine, sawtooth, square, triangle).

The transition matrix is musically motivated — sine and triangle are "neighbors", sawtooth and square are "neighbors". Transitions are probabilistic and smooth via crossfade.

| Control | Effect |
|---------|--------|
| **SHAPE** | Chaos — transition probability (0 = frozen, max = hyper-chaotic) |
| **SHIFT+SHAPE** | Morph speed — slow = smooth crossfade, fast = instant snap |
| **Param 1** — SubMix (0–100%) | Sub-oscillator level |
| **Param 2** — SubInt (0–2) | Sub interval: 0 = octave↓, 1 = fifth↓, 2 = unison+detune |
| **Param 3** — Bias (0–4) | Preferred waveform: 0 = balanced, 1 = sine, 2 = saw, 3 = square, 4 = tri |

Designed to pair with **[BANG!](https://github.com/obareau/bang)** — use BANG!'s NTS-1 mode to send per-step CC locks (cutoff, osc shape, LFO depth) while MarkovWave handles the timbre evolution.

---

## Effects — modfx slot

*The modfx slot processes audio before the built-in delay and reverb.*

### FuzzStack _(coming)_

> Three fuzz stages in series, each with a distinct character.

**Architecture:**
```
input → [Stage 1: Soft Fuzz] → [Stage 2: Hard Clip] → [Stage 3: Octave/Gate] → tone filter → output
```

| Control | Effect |
|---------|--------|
| **TIME** | Drive — global pre-gain |
| **DEPTH** | Stack depth — blend 1 → 2 → 3 stages |

| Stage | Algorithm | Character |
|-------|-----------|-----------|
| **Soft Fuzz** | `tanh(gain × x)` | Warm, tube-like, even harmonics |
| **Hard Clip** | `clip(x, ±threshold)` | Transistor, aggressive mids |
| **Octave Fuzz** | Full-wave rectification | Octave-up, Octavia-style, screaming |

Shift+Shape modes: asymmetric clip (fuzz-face vintage) · gated fuzz (dying battery) · bit-reduce post-stage (lo-fi crunch)

---

### VariTape _(coming)_

> Tape machine simulation — wow, flutter, saturation, speed drift.

Models the mechanical instability of a tape transport. Implemented as a short delay buffer with LFO-modulated read head position + soft saturation on the write path.

| Control | Effect |
|---------|--------|
| **TIME** | Tape age — overall wow+flutter intensity |
| **DEPTH** | Dry/wet mix |

- **Wow** (slow LFO ~0.3–2 Hz) → slow pitch drift, like a tape starting up
- **Flutter** (fast LFO ~7–15 Hz) → mechanical vibration, capstan irregularity
- **Saturation** (pre-write) → tape compression + harmonic warmth
- Shift+Shape: varispeed mode — sustained pitch shift up or down (slowing/speeding the "tape")

*Implementation note: fits in modfx slot; requires a ~500ms circular delay buffer.*

---

### RoboVox _(coming)_

> Robotic voice effect — ring modulation + comb filter formants.

A full vocoder requires FFT analysis (too heavy for NTS-1 mk1). RoboVox achieves the robotic voice character through a chain of ring modulation and resonant comb filters tuned to vowel formants, without any FFT.

| Control | Effect |
|---------|--------|
| **TIME** | Carrier frequency — the "pitch" of the robot voice |
| **DEPTH** | Formant intensity — how strong the vowel resonances are |

**Architecture:**
```
input → ring mod (carrier sine) → comb filter bank (F1/F2/F3 formants) → output
```

- Ring mod at TIME frequency → classic robot/Dalek character
- Comb filters at fixed formant ratios → adds vowel shape on top
- Shift+Shape: selects vowel preset (A / E / I / O / U) → shifts formant positions

*Works best on voice or monophonic synth. On pads: creates alien choir textures.*

---

## Effects — revfx slot

*The revfx slot is the last in the chain, after modfx and delfx.*

### SpringVerb _(coming)_

> Spring reverb emulation — the classic boingy tank sound.

Spring reverbs have a distinctive character from dispersive wave propagation: high frequencies travel faster than low ones, creating the characteristic "splash" on transients.

| Control | Effect |
|---------|--------|
| **TIME** | Spring tension — decay length and resonance |
| **DEPTH** | Wet mix |

**Architecture:**
```
input → all-pass cascade (dispersion) → comb filter (resonance) → high-freq boost → output
```

- All-pass filters with frequency-dependent delay → dispersion = the "boing"
- Feedback comb filter → sustain and metallic ring
- High shelf boost → spring's characteristic bright splash
- Shift+Shape: spring mass (heavier = more low-end resonance, slower)

*3–5 all-pass stages + 2 comb filters — feasible within NTS-1 mk1 RAM constraints.*

---

### Shimmer _(coming)_

> Reverb with pitch-shifted feedback — the classic ambient/post-rock shimmer.

A reverb whose feedback path includes a pitch shifter (typically +1 octave), creating an ethereal, rising shimmer effect as the signal decays.

| Control | Effect |
|---------|--------|
| **TIME** | Reverb size / decay |
| **DEPTH** | Shimmer amount — how much octave-up is fed back |

**Architecture:**
```
input → all-pass reverb → [feedback path: pitch shift +1oct] → mix → output
                                    ↑_________________________________|
```

- Reverb: all-pass network (Schroeder-style, light)
- Pitch shift: granular overlap-add at ×2 (octave up) — simplified, works on harmonic material
- Shift+Shape: pitch interval (octave up / fifth up / octave+fifth)

*Implementation note: pitch shifting on embedded is CPU-intensive. Grain size ~20ms, 2 grains alternating = feasible at 48kHz on Cortex-M4 with FPU.*

---

## Roadmap

### Oscillators
- [x] **MarkovWave** — Markov chain waveform morpher
- [ ] **Drone** — slow harmonic random walk, ambient textures
- [ ] **Karplus-Strong** — physical string synthesis
- [ ] **Bitfield** — bitwise operations on samples, lo-fi glitch
- [ ] **PM Stack** — phase modulation with variable carrier/modulator ratio

### Effects (modfx)
- [ ] **FuzzStack** — 3-stage fuzz: soft → hard clip → octave, TIME=drive, DEPTH=stages
- [ ] **VariTape** — wow/flutter/saturation tape emulation, varispeed mode
- [ ] **RoboVox** — ring mod + comb formants robot voice, no FFT

### Effects (revfx)
- [ ] **SpringVerb** — dispersive all-pass cascade + comb, boingy spring character
- [ ] **Shimmer** — reverb with granular octave-up feedback, ambient post-rock

---

## Build

**Requirements:**
- `arm-none-eabi-gcc` (v10+)
- [logue SDK](https://github.com/korginc/logue-sdk) with submodules initialized

```bash
# Clone logue SDK once
git clone --depth=1 https://github.com/korginc/logue-sdk.git
cd logue-sdk && git submodule update --init --recursive

# Build any plugin
cd nts1-plugins/<plugin-name>
PLATFORMDIR=/path/to/logue-sdk/platform/nutekt-digital make install
# → <plugin-name>.ntkdigunit
```

**Install:** Open Korg NTS-1 Librarian → User Osc / Mod FX / Rev FX slot → drag `.ntkdigunit`.

### Ubuntu / Debian
```bash
sudo apt install gcc-arm-none-eabi
```

### macOS
```bash
brew install --cask gcc-arm-embedded
```

---

## Project structure

```
nts1-plugins/
├── markovwave/    # osc   — Markov chain waveform morpher       ✅
├── fuzzstack/     # modfx — 3-stage fuzz stack
├── varitape/      # modfx — tape wow/flutter/varispeed
├── robovox/       # modfx — ring mod + formant robot voice
├── springverb/    # revfx — spring reverb emulation
└── shimmer/       # revfx — pitch-shifted reverb shimmer
```

---

## License

MIT

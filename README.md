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

## Effects (modfx)

### FuzzStack _(coming)_

> Three fuzz stages in series, each with a distinct character. Stack them or use individually.

The NTS-1 modfx slot processes audio before the built-in delay and reverb, making it ideal for distortion and fuzz.

**Architecture:**
```
input → [Stage 1: Soft Fuzz] → [Stage 2: Hard Clip] → [Stage 3: Octave/Gate] → tone filter → output
```

| Control | Effect |
|---------|--------|
| **TIME** | Drive — global pre-gain across all active stages |
| **DEPTH** | Stack depth — how many stages are active (blend 1→2→3) |

**The three stages:**

| Stage | Algorithm | Character |
|-------|-----------|-----------|
| **Soft Fuzz** | `tanh(gain × x)` | Warm, tube-like, even harmonics |
| **Hard Clip** | `clip(x, ±threshold)` | Transistor brutal, aggressive mids |
| **Octave Fuzz** | Full-wave rectification | Octave-up (Octavia-style), screaming |

**Bonus modes via Shift+Shape:**
- Asymmetric clip (different +/- thresholds) → vintage, odd harmonics, almost fuzz-face
- Gated fuzz (noise gate + hard clip) → sputtery dying-battery sound
- Bit reduce post-stage → lo-fi digital crunch on top of analog-ish fuzz

---

## Roadmap

### Oscillators
- [x] **MarkovWave** — Markov chain waveform morpher
- [ ] **Drone** — slow harmonic random walk, ambient textures
- [ ] **Karplus-Strong** — physical string synthesis
- [ ] **Bitfield** — bitwise operations on samples, lo-fi glitch
- [ ] **PM Stack** — phase modulation with variable carrier/modulator ratio
- [ ] **PolyBLEP Saw** — band-limited sawtooth reference oscillator

### Effects (modfx)
- [ ] **FuzzStack** — 3-stage fuzz (soft → hard clip → octave), TIME=drive, DEPTH=stages

### Effects (delfx / revfx)
- [ ] TBD

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

**Install:** Open Korg NTS-1 Librarian → User Osc or Mod FX slot → drag `.ntkdigunit`.

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
├── markovwave/       # oscillator — Markov chain waveform morpher
├── fuzzstack/        # modfx — 3-stage fuzz (coming)
└── ...
```

Each plugin follows the `dummy-osc` / `dummy-modfx` template from the logue SDK.

---

## License

MIT

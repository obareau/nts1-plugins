# NTS-1 Oscillators

Custom user oscillators for the **Korg NTS-1 mk1** (nutekt-digital platform, logue SDK v1).

Each oscillator is a standalone project that compiles to a `.ntkdigunit` file, ready to load via the Korg NTS-1 Librarian.

---

## Oscillators

### MarkovWave

> A Markov chain morphs between four waveforms (sine, sawtooth, square, triangle).

The transition matrix is musically motivated — sine and triangle are "neighbors", sawtooth and square are "neighbors". Transitions are probabilistic and smooth via crossfade.

| Control | Effect |
|---------|--------|
| **SHAPE** | Chaos — transition probability (0 = frozen, max = hyper-chaotic) |
| **SHIFT+SHAPE** | Morph speed — slow = smooth crossfade, fast = instant snap |
| **Param 1** — SubMix (0–100%) | Sub-oscillator level |
| **Param 2** — SubInt (0–2) | Sub interval: 0 = octave↓, 1 = fifth↓, 2 = unison+detune |
| **Param 3** — Bias (0–4) | Preferred waveform: 0 = balanced, 1 = sine, 2 = saw, 3 = square, 4 = tri |

Designed to pair with **[BANG!](https://github.com/obareau/bang)** — the Markov sequencer. Use BANG!'s NTS-1 mode to send per-step CC locks (cutoff, osc shape, LFO depth) while MarkovWave handles the timbre evolution.

---

## Build

**Requirements:**
- `arm-none-eabi-gcc` (v10+)
- [logue SDK](https://github.com/korginc/logue-sdk) (with submodules)

```bash
# Clone logue SDK once
git clone --depth=1 https://github.com/korginc/logue-sdk.git
cd logue-sdk && git submodule update --init --recursive

# Build an oscillator
cd nts1-oscillators/markovwave
PLATFORMDIR=/path/to/logue-sdk/platform/nutekt-digital make install
# → markovwave.ntkdigunit
```

**Install:** Open Korg NTS-1 Librarian → User Osc slot → drag `.ntkdigunit`.

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

Each oscillator follows this layout (based on `dummy-osc` template):

```
markovwave/
├── manifest.json    # metadata, parameter names and ranges
├── project.mk       # project name, source files, toolchain override
├── Makefile         # from logue SDK dummy-osc (unmodified)
├── tpl/             # from logue SDK dummy-osc (unmodified)
├── ld/              # from logue SDK dummy-osc (unmodified)
└── markovwave.c     # oscillator code
```

---

## Roadmap

- [x] **MarkovWave** — Markov chain waveform morpher
- [ ] **Drone** — slow harmonic random walk, ambient textures
- [ ] **Karplus-Strong** — physical string synthesis
- [ ] **Bitfield** — bitwise operations on samples, lo-fi glitch
- [ ] **PM Stack** — phase modulation with variable carrier/modulator ratio

---

## License

MIT

/*
 * MarkovWave — oscillateur Korg NTS-1 (logue SDK v1)
 *
 * Morphe entre 4 formes d'onde (sine / sawtooth / square / triangle)
 * via une chaîne de Markov. Conçu pour BANG! — même philosophie.
 *
 * SHAPE       → chaos : probabilité de transition [0 = stable, max = hyper]
 * SHIFT+SHAPE → morph speed : vitesse du crossfade entre formes
 * Param 1     → sub mix : niveau du sub-oscillateur (octave basse)
 * Param 2     → sub interval : 0 = octave, 1 = quinte, 2 = unisson (détune léger)
 * Param 3     → waveform bias : 0 = équilibré, 1 = ♡ sine, 2 = ♡ saw,
 *                                               3 = ♡ square, 4 = ♡ triangle
 */

#include "userosc.h"

/* ── waveforms ──────────────────────────────────────────────────────────── */

#define WF_SINE  0
#define WF_SAW   1
#define WF_SQR   2
#define WF_TRI   3
#define WF_COUNT 4

/* ── Markov transition matrix [from][to] ─────────────────────────────────
 * Valeurs choisies pour que les transitions soient musicalement naturelles :
 * sine↔triangle sont proches, square↔saw sont proches.
 * Lignes normalisées à 1.0.
 */
static const float k_trans[WF_COUNT][WF_COUNT] = {
  /* from SINE   */ { 0.40f, 0.18f, 0.08f, 0.34f },
  /* from SAW    */ { 0.18f, 0.35f, 0.32f, 0.15f },
  /* from SQUARE */ { 0.08f, 0.34f, 0.36f, 0.22f },
  /* from TRI    */ { 0.36f, 0.14f, 0.14f, 0.36f },
};

/* Biais par param3 : boost de la diagonale de la waveform cible */
static const float k_bias_boost = 0.25f;  /* ajout sur la waveform biaisée */

/* ── state ──────────────────────────────────────────────────────────────── */

typedef struct {
  float    phi;          /* phase principale [0, 1) */
  float    phi_sub;      /* phase sub-oscillateur */
  float    w0;           /* incrément de phase par sample */
  uint8_t  wf_cur;       /* waveform courante */
  uint8_t  wf_next;      /* waveform cible */
  float    morph;        /* position du crossfade [0, 1] */
  float    morph_speed;  /* incrément morph par sample */
  float    chaos;        /* probabilité de transition */
  float    sub_mix;      /* niveau sub-osc [0, 1] */
  uint8_t  sub_interval; /* 0=oct, 1=quinte, 2=détune */
  uint8_t  bias_wf;      /* 0=none, 1-4=waveform biaisée */
  uint32_t lfsr;         /* générateur pseudo-aléatoire */
  uint8_t  reset;        /* flag note-on → reset phase */
} State;

static State s;

/* ── LFSR xorshift32 (Galois) ─────────────────────────────────────────── */
static inline uint32_t lfsr_next(void)
{
  s.lfsr ^= s.lfsr << 13;
  s.lfsr ^= s.lfsr >> 17;
  s.lfsr ^= s.lfsr << 5;
  return s.lfsr;
}

/* float [0, 1) depuis LFSR */
static inline float rand01(void)
{
  return (float)(lfsr_next() & 0x7FFFFFFFu) * 4.65661287e-10f;
}

/* ── sample de waveform ──────────────────────────────────────────────────
 * Toutes les formes sont anti-aliasées trivialement (pas de PolyBLEP ici,
 * suffisant pour usage percussif/texturel). Phase en [0, 1).
 */
static inline float wf_sample(uint8_t wf, float phi)
{
  switch (wf) {
    case WF_SINE:
      return osc_sinf(phi);
    case WF_SAW:
      return 2.f * phi - 1.f;
    case WF_SQR:
      return phi < 0.5f ? 0.75f : -0.75f;
    case WF_TRI:
    default:
      return phi < 0.5f ? (4.f * phi - 1.f) : (3.f - 4.f * phi);
  }
}

/* ── sélection Markov ────────────────────────────────────────────────────
 * Tire la prochaine waveform depuis la ligne `cur` de la matrice,
 * après application du biais et du chaos.
 */
static uint8_t markov_next(uint8_t cur)
{
  /* Avec probabilité (1 - chaos), on reste sur la waveform courante */
  if (rand01() > s.chaos) return cur;

  /* Copie la ligne de transition + boost biais */
  float row[WF_COUNT];
  float total = 0.f;
  for (uint8_t i = 0; i < WF_COUNT; i++) {
    row[i] = k_trans[cur][i];
    if (s.bias_wf > 0 && i == (s.bias_wf - 1))
      row[i] += k_bias_boost;
    total += row[i];
  }

  /* Tirage pondéré */
  float r = rand01() * total;
  float acc = 0.f;
  for (uint8_t i = 0; i < WF_COUNT; i++) {
    acc += row[i];
    if (r < acc) return i;
  }
  return cur;
}

/* ── incrément sub-oscillateur ───────────────────────────────────────────
 * 0 = octave basse (w0 * 0.5)
 * 1 = quinte basse  (w0 * 0.5 * 2/3 ≈ w0 * 0.3333)
 * 2 = unisson légèrement désaccordé (w0 * 1.003)
 */
static inline float sub_w0(void)
{
  switch (s.sub_interval) {
    case 0:  return s.w0 * 0.5f;
    case 1:  return s.w0 * 0.3333f;
    case 2:  return s.w0 * 1.0029f;  /* ~5 cents de détune */
    default: return s.w0 * 0.5f;
  }
}

/* ── API logue SDK ───────────────────────────────────────────────────────── */

void OSC_INIT(uint32_t platform, uint32_t api)
{
  (void)platform;
  (void)api;
  s.phi          = 0.f;
  s.phi_sub      = 0.f;
  s.w0           = 0.f;
  s.wf_cur       = WF_SINE;
  s.wf_next      = WF_TRI;
  s.morph        = 0.f;
  s.morph_speed  = 0.0015f;
  s.chaos        = 0.25f;
  s.sub_mix      = 0.f;
  s.sub_interval = 0;
  s.bias_wf      = 0;
  s.lfsr         = 0xDEADBEEFu;
  s.reset        = 0;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn, const uint32_t frames)
{
  /* Mise à jour pitch */
  s.w0 = osc_w0f_for_note((params->pitch) >> 8, params->pitch & 0xFF);

  /* SHAPE → chaos [0.02 .. 0.98] */
  const float shape = param_val_to_f32(params->shape_lfo);
  s.chaos = clipminmaxf(0.02f, shape, 0.98f);

  /* Reset phase sur note-on */
  if (s.reset) {
    s.phi     = 0.f;
    s.phi_sub = 0.f;
    s.morph   = 0.f;
    s.reset   = 0;
  }

  const float sw0_sub = sub_w0();

  q31_t * __restrict y = (q31_t *)yn;
  const q31_t * y_e    = y + frames;

  for (; y != y_e; y++) {
    /* Sample crossfadé entre wf_cur et wf_next */
    const float s_cur  = wf_sample(s.wf_cur,  s.phi);
    const float s_next = wf_sample(s.wf_next, s.phi);
    float sig          = linintf(s.morph, s_cur, s_next);

    /* Sub-oscillateur */
    if (s.sub_mix > 0.01f) {
      const float sub = wf_sample(s.wf_cur, s.phi_sub);
      sig = linintf(s.sub_mix, sig, sub);
    }

    /* Avance le morphing */
    s.morph += s.morph_speed;
    if (s.morph >= 1.f) {
      s.morph   = 0.f;
      s.wf_cur  = s.wf_next;
      s.wf_next = markov_next(s.wf_cur);
    }

    /* Avance les phases */
    s.phi += s.w0;
    s.phi -= (uint32_t)s.phi;

    s.phi_sub += sw0_sub;
    s.phi_sub -= (uint32_t)s.phi_sub;

    /* Soft clip + sortie */
    sig  = osc_softclipf(0.08f, sig);
    *y   = f32_to_q31(sig);
  }
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  (void)params;
  s.reset = 1;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  switch (index) {

    case k_user_osc_param_shape:
      /* géré dans OSC_CYCLE via params->shape_lfo */
      break;

    case k_user_osc_param_shiftshape: {
      /* morph speed : quadratique pour plus de contrôle aux valeurs faibles */
      const float t = param_val_to_f32(value);          /* [0, ~1] */
      s.morph_speed = 0.0001f + t * t * 0.025f;
      break;
    }

    case k_user_osc_param_id1:
      /* Param 1 : sub mix 0–100 % */
      s.sub_mix = clipminmaxf(0.f, value * 0.01f, 1.f);
      break;

    case k_user_osc_param_id2:
      /* Param 2 : sub interval 0-2 */
      s.sub_interval = (uint8_t)(value % 3);
      break;

    case k_user_osc_param_id3:
      /* Param 3 : waveform bias 0-4 */
      s.bias_wf = (uint8_t)(value % 5);
      break;

    default:
      break;
  }
}

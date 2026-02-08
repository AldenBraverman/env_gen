/**
 * Parameter IDs and metadata (must match plugin createParameterLayout).
 * State is always linear normalized 0–1. Display = real value.
 */
export interface ParamMeta {
  id: string;
  label: string;
  min?: number;
  max?: number;
  step?: number;
  unit?: string;
  type: "float" | "bool" | "choice";
  choices?: string[];
  skew?: number;
}

const RATE_CHOICES = ["1/1", "1/2", "1/4", "1/8", "1/16", "1/32"];

const stepIds = Array.from({ length: 16 }, (_, i) => `lane1_step${i}`);

export const PARAMS: ParamMeta[] = [
  { id: "inputGain", label: "Input Gain", min: -24, max: 24, step: 0.1, unit: "dB", type: "float" },
  { id: "outputGain", label: "Output Gain", min: -24, max: 24, step: 0.1, unit: "dB", type: "float" },
  { id: "dryPass", label: "Dry", type: "bool" },
  ...stepIds.map((id, i) => ({ id, label: `Step ${i + 1}`, type: "bool" as const })),
  { id: "lane1_attack", label: "Attack", min: 0.001, max: 10, step: 0.001, unit: "s", type: "float", skew: 0.3 },
  { id: "lane1_hold", label: "Hold", min: 0, max: 10, step: 0.001, unit: "s", type: "float", skew: 0.3 },
  { id: "lane1_decay", label: "Decay", min: 0.001, max: 10, step: 0.001, unit: "s", type: "float", skew: 0.3 },
  { id: "lane1_rate", label: "Rate", type: "choice", choices: RATE_CHOICES },
  { id: "lane1_destination", label: "Assign", type: "choice", choices: ["None", "Amplitude"] },
  { id: "lane1_amount", label: "Amount", min: -1, max: 1, step: 0.01, type: "float" },
];

export function getParamMeta(id: string): ParamMeta | undefined {
  return PARAMS.find((p) => p.id === id);
}

/** Normalised 0–1 → real value. */
export function normalizedToReal(meta: ParamMeta, n: number): number {
  if (meta.type === "float" && meta.min != null && meta.max != null)
    return meta.min + (meta.max - meta.min) * Math.max(0, Math.min(1, n));
  if (meta.type === "choice" && meta.choices?.length)
    return Math.round(n * (meta.choices.length - 1));
  return n;
}

/** Real value → normalised 0–1. */
export function realToNormalized(meta: ParamMeta, value: number): number {
  if (meta.type === "float" && meta.min != null && meta.max != null) {
    const range = meta.max - meta.min;
    if (range === 0) return 0;
    return Math.max(0, Math.min(1, (value - meta.min) / range));
  }
  if (meta.type === "choice" && meta.choices?.length)
    return meta.choices.length <= 1 ? 0 : value / (meta.choices.length - 1);
  return value;
}

/** For slider display: normalised → 0–1 position (with optional skew). */
export function realToSliderPosition(meta: ParamMeta, real: number): number {
  if (meta.type !== "float" || meta.min == null || meta.max == null) return real;
  const { min, max, skew = 1 } = meta;
  const range = max - min;
  if (range === 0) return 0;
  const proportion = Math.max(0, Math.min(1, (real - min) / range));
  if (skew !== 1 && proportion > 0) return Math.pow(proportion, skew);
  return proportion;
}

/** Slider position 0–1 → real. */
export function sliderPositionToReal(meta: ParamMeta, pos: number): number {
  if (meta.type !== "float" || meta.min == null || meta.max == null) return pos;
  const { min, max, skew = 1 } = meta;
  const range = max - min;
  const p = Math.max(0, Math.min(1, pos));
  if (skew !== 1 && p > 0) {
    const mapped = Math.exp(Math.log(p) / skew);
    return min + range * mapped;
  }
  return min + range * p;
}

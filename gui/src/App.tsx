import { useEffect, useState, useCallback, useRef } from "react";
import {
  getState,
  setParameter,
  setEnvGenCallbacks,
  resetAllParameters,
} from "./lib/bridge";
import {
  getParamMeta,
  normalizedToReal,
  realToNormalized,
  realToSliderPosition,
  sliderPositionToReal,
  type ParamMeta,
} from "./lib/params";
import { SECTIONS } from "./lib/sections";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Label } from "@/components/ui/label";
import { Slider } from "@/components/ui/slider";
import { Switch } from "@/components/ui/switch";
import { Button } from "@/components/ui/button";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";

type State = Record<string, number>;

function laneParamIdsForLane(laneNum: number): string[] {
  const ids: string[] = [];
  for (let i = 0; i < 16; i++) ids.push(`lane${laneNum}_step${i}`);
  ids.push(`lane${laneNum}_attack`, `lane${laneNum}_hold`, `lane${laneNum}_decay`, `lane${laneNum}_rate`, `lane${laneNum}_destination`, `lane${laneNum}_amount`);
  return ids;
}

/** Fixed palette for lane colours (index 0..7). Must match C++ OscilloscopeComponent::getLaneColour. */
export const LANE_COLOURS = [
  "#00ffaa", // 0: cyan-green
  "#ff8c00", // 1: orange
  "#4da6ff", // 2: blue
  "#e040fb", // 3: magenta
  "#ffeb3b", // 4: yellow
  "#26a69a", // 5: teal
  "#ff7043", // 6: coral
  "#b39ddb", // 7: lavender
];

function ParamControl({
  meta,
  value,
  onChange,
  onDragStart,
  onDragEnd,
}: {
  meta: ParamMeta;
  value: number;
  onChange: (n: number) => void;
  onDragStart?: (paramId: string) => void;
  onDragEnd?: () => void;
}) {
  const real = normalizedToReal(meta, value);

  const handleChange = (newReal: number) => {
    const n = realToNormalized(meta, newReal);
    onChange(n);
    setParameter(meta.id, n);
  };

  if (meta.type === "bool") {
    return (
      <div className="flex items-center gap-2">
        <Switch
          checked={value >= 0.5}
          onCheckedChange={(checked) => handleChange(checked ? 1 : 0)}
        />
        <Label className="text-sm text-muted-foreground">{meta.label}</Label>
      </div>
    );
  }

  if (meta.type === "choice" && meta.choices) {
    const index = Math.round(value * (meta.choices.length - 1));
    const clampedIndex = Math.max(0, Math.min(index, meta.choices.length - 1));
    return (
      <div className="flex flex-col gap-1">
        <Label className="text-xs text-muted-foreground">{meta.label}</Label>
        <Select
          value={String(clampedIndex)}
          onValueChange={(v) => handleChange(Number(v))}
        >
          <SelectTrigger className="h-8 w-full">
            <SelectValue />
          </SelectTrigger>
          <SelectContent>
            {meta.choices.map((c, i) => (
              <SelectItem key={c} value={String(i)}>
                {c}
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>
    );
  }

  return (
    <div className="flex flex-col gap-1">
      <Label className="text-xs text-muted-foreground">
        {meta.label}
        {meta.unit != null ? ` (${meta.unit})` : ""}
      </Label>
      <div className="flex items-center gap-2">
        <Slider
          className="w-24 flex-1"
          min={0}
          max={1}
          step={0.001}
          value={[realToSliderPosition(meta, real)]}
          onValueChange={([v]) =>
            handleChange(sliderPositionToReal(meta, v ?? 0))
          }
          onPointerDown={() => onDragStart?.(meta.id)}
          onPointerUp={() => onDragEnd?.()}
          onPointerLeave={() => onDragEnd?.()}
        />
        <span className="w-14 text-right text-sm tabular-nums text-foreground">
          {typeof real === "number" && !Number.isInteger(real)
            ? real.toFixed(2)
            : real}
          {meta.unit ?? ""}
        </span>
      </div>
    </div>
  );
}

function EnvelopeSection({
  state,
  setStateParam,
  onDragStart,
  onDragEnd,
  getParamMeta,
}: {
  state: State;
  setStateParam: (id: string, n: number) => void;
  onDragStart: (paramId: string) => void;
  onDragEnd: () => void;
  getParamMeta: (id: string) => ParamMeta | undefined;
}) {
  const numLanesNorm = state["numLanes"] ?? 0;
  const numLanes = Math.round(normalizedToReal(getParamMeta("numLanes") ?? { id: "", label: "", type: "float", min: 0, max: 8 }, numLanesNorm));
  const safeNumLanes = Math.max(0, Math.min(8, numLanes));

  const handleAddLane = () => {
    if (safeNumLanes >= 8) return;
    const next = safeNumLanes + 1;
    setStateParam("numLanes", next / 8);
  };

  const handleRemoveLane = (removeIndex: number) => {
    if (safeNumLanes <= 0 || removeIndex < 0 || removeIndex >= safeNumLanes) return;
    // Copy lane (j+2) -> lane (j+1) for j = removeIndex .. numLanes-2 so we read from untouched lanes
    for (let j = removeIndex; j <= safeNumLanes - 2; j++) {
      const srcLane = j + 2;
      const destLane = j + 1;
      const srcIds = laneParamIdsForLane(srcLane);
      const destIds = laneParamIdsForLane(destLane);
      srcIds.forEach((id, idx) => {
        const val = state[id] ?? 0;
        setStateParam(destIds[idx], val);
      });
    }
    setStateParam("numLanes", (safeNumLanes - 1) / 8);
  };

  if (safeNumLanes === 0) {
    return (
      <div className="flex items-center gap-2">
        <Button variant="outline" size="sm" onClick={handleAddLane} aria-label="Add lane">
          +
        </Button>
        <span className="text-xs text-muted-foreground">Add lane</span>
      </div>
    );
  }

  return (
    <div className="space-y-4">
      <div className="flex items-center gap-2">
        <Button
          variant="outline"
          size="sm"
          onClick={handleAddLane}
          disabled={safeNumLanes >= 8}
          aria-label="Add lane"
        >
          +
        </Button>
        <span className="text-xs text-muted-foreground">
          {safeNumLanes} lane{safeNumLanes !== 1 ? "s" : ""}
        </span>
      </div>
      {Array.from({ length: safeNumLanes }, (_, i) => {
        const laneNum = i + 1;
        const stepIds = Array.from({ length: 16 }, (_, s) => `lane${laneNum}_step${s}`);
        const envIds = [`lane${laneNum}_attack`, `lane${laneNum}_hold`, `lane${laneNum}_decay`, `lane${laneNum}_rate`, `lane${laneNum}_destination`, `lane${laneNum}_amount`];
        const laneColor = LANE_COLOURS[i] ?? LANE_COLOURS[0];
        return (
          <div
            key={laneNum}
            className="rounded-md border border-border bg-muted/30 p-3 space-y-2"
            style={{ borderLeftWidth: 4, borderLeftColor: laneColor }}
          >
            <div className="flex items-center justify-between gap-2">
              <span className="text-xs font-medium" style={{ color: laneColor }}>
                Lane {laneNum}
              </span>
              <Button
                variant="ghost"
                size="icon"
                className="h-7 w-7 text-muted-foreground hover:text-destructive"
                onClick={() => handleRemoveLane(i)}
                aria-label={`Remove lane ${laneNum}`}
              >
                -
              </Button>
            </div>
            <div className="flex flex-wrap items-center gap-1">
              <Label className="mr-2 w-12 text-xs text-muted-foreground">Steps</Label>
              {stepIds.map((paramId) => {
                const meta = getParamMeta(paramId);
                if (!meta) return null;
                const stepIndex = parseInt(paramId.replace(`lane${laneNum}_step`, ""), 10);
                return (
                  <Button
                    key={paramId}
                    variant={(state[paramId] ?? 0) >= 0.5 ? "default" : "outline"}
                    size="icon"
                    className="h-8 w-8"
                    onClick={() =>
                      setStateParam(paramId, (state[paramId] ?? 0) >= 0.5 ? 0 : 1)
                    }
                  >
                    {stepIndex + 1}
                  </Button>
                );
              })}
            </div>
            <div className="grid grid-cols-2 gap-3 sm:grid-cols-3 md:grid-cols-5">
              {envIds.map((paramId) => {
                const meta = getParamMeta(paramId);
                if (!meta) return null;
                return (
                  <ParamControl
                    key={paramId}
                    meta={meta}
                    value={state[paramId] ?? 0}
                    onChange={(n) => setStateParam(paramId, n)}
                    onDragStart={onDragStart}
                    onDragEnd={onDragEnd}
                  />
                );
              })}
            </div>
          </div>
        );
      })}
    </div>
  );
}

export default function App() {
  const [state, setState] = useState<State>({});
  const draggingParamIdRef = useRef<string | null>(null);

  const updateParam = useCallback((id: string, value: number) => {
    setState((s) => ({ ...s, [id]: value }));
  }, []);

  useEffect(() => {
    setEnvGenCallbacks({
      updateParams: (id: string, value: number) => {
        if (id === draggingParamIdRef.current) return;
        updateParam(id, value);
      },
    });
    getState().then((s) => setState(s));
  }, [updateParam]);

  const setStateParam = useCallback((id: string, n: number) => {
    setState((s) => ({ ...s, [id]: n }));
    setParameter(id, n);
  }, []);

  const onDragStart = useCallback((paramId: string) => {
    draggingParamIdRef.current = paramId;
  }, []);
  const onDragEnd = useCallback(() => {
    draggingParamIdRef.current = null;
  }, []);

  return (
    <div className="flex min-h-screen flex-col bg-background p-3 text-foreground">
      <div className="mb-3 flex items-center justify-center rounded-lg border border-border bg-card px-4 py-2.5">
        <h1 className="text-lg font-semibold tracking-wide text-accent">
          Envelope Generator
        </h1>
      </div>

      <div className="flex flex-1 flex-col gap-3">
        {SECTIONS.map((section) => (
          <Card key={section.id}>
            <CardHeader className="pb-2">
              <CardTitle className="text-sm">{section.title}</CardTitle>
            </CardHeader>
            <CardContent className="space-y-3">
              {section.id === "ENVELOPE" ? (
                <EnvelopeSection
                  state={state}
                  setStateParam={setStateParam}
                  onDragStart={onDragStart}
                  onDragEnd={onDragEnd}
                  getParamMeta={getParamMeta}
                />
              ) : (
                <div className="grid grid-cols-1 gap-3 sm:grid-cols-3">
                  {section.paramIds.map((paramId) => {
                    const meta = getParamMeta(paramId);
                    if (!meta) return null;
                    return (
                      <ParamControl
                        key={paramId}
                        meta={meta}
                        value={state[paramId] ?? 0}
                        onChange={(n) => setStateParam(paramId, n)}
                        onDragStart={onDragStart}
                        onDragEnd={onDragEnd}
                      />
                    );
                  })}
                </div>
              )}
            </CardContent>
          </Card>
        ))}
      </div>

      <div className="mt-2 flex justify-end gap-2">
        <Button
          variant="outline"
          size="sm"
          onClick={() => resetAllParameters()}
          title="Set all parameters to default values"
        >
          Reset All
        </Button>
        <Button
          variant="outline"
          size="sm"
          onClick={() => window.location.reload()}
          title="Reload UI and sync with current plugin state"
        >
          Refresh
        </Button>
      </div>
    </div>
  );
}

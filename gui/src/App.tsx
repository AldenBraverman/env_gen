import { useEffect, useState, useCallback, useRef } from "react";
import {
  getState,
  setParameter,
  setEnvGenCallbacks,
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
                <>
                  <div className="flex flex-wrap items-center gap-1">
                    <Label className="mr-2 w-12 text-xs text-muted-foreground">
                      Steps
                    </Label>
                    {section.paramIds
                      .filter((id) => id.startsWith("lane1_step"))
                      .map((paramId) => {
                        const meta = getParamMeta(paramId);
                        if (!meta) return null;
                        const stepIndex = parseInt(
                          paramId.replace("lane1_step", ""),
                          10
                        );
                        return (
                          <Button
                            key={paramId}
                            variant={state[paramId] >= 0.5 ? "default" : "outline"}
                            size="icon"
                            className="h-8 w-8"
                            onClick={() =>
                              setStateParam(
                                paramId,
                                (state[paramId] ?? 0) >= 0.5 ? 0 : 1
                              )
                            }
                          >
                            {stepIndex + 1}
                          </Button>
                        );
                      })}
                  </div>
                  <div className="grid grid-cols-2 gap-3 sm:grid-cols-3 md:grid-cols-5">
                    {section.paramIds
                      .filter((id) => !id.startsWith("lane1_step"))
                      .map((paramId) => {
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
                </>
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

      <div className="mt-2 flex justify-end">
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

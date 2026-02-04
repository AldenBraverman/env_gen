/**
 * Bridge to JUCE native backend (setParameter, getState).
 * C++ pushes updates via window.__ENVGEN__.updateParams(id, value) — set from App.
 */

declare global {
  interface Window {
    __JUCE__?: {
      backend?: {
        emitEvent: (eventId: string, payload: unknown) => void;
        addEventListener: (eventId: string, fn: (payload: unknown) => void) => [string, number];
      };
      initialisationData?: {
        __juce__functions?: string[];
      };
    };
    __ENVGEN__?: {
      updateParams?: (id: string, value: number) => void;
    };
  }
}

const INVOKE = "__juce__invoke";
const COMPLETE = "__juce__complete";

let promiseId = 0;
const pending = new Map<number, { resolve: (v: unknown) => void; reject: (e: unknown) => void }>();

function ensureListener(): void {
  if (typeof window.__JUCE__?.backend?.addEventListener !== "function") return;
  window.__JUCE__.backend.addEventListener(COMPLETE, (payload: unknown) => {
    const { promiseId: id, result } = payload as { promiseId: number; result: unknown };
    if (pending.has(id)) {
      pending.get(id)!.resolve(result);
      pending.delete(id);
    }
  });
}

function invoke(name: string, ...params: unknown[]): Promise<unknown> {
  return new Promise((resolve, reject) => {
    if (typeof window.__JUCE__?.backend?.emitEvent !== "function") {
      reject(new Error("JUCE backend not available"));
      return;
    }
    ensureListener();
    const id = promiseId++;
    pending.set(id, { resolve, reject });
    window.__JUCE__!.backend!.emitEvent(INVOKE, {
      name,
      params,
      resultId: id,
    });
  });
}

export async function getState(): Promise<Record<string, number>> {
  const result = await invoke("getState");
  if (result != null && typeof result === "object" && !Array.isArray(result)) {
    return result as Record<string, number>;
  }
  return {};
}

export function setParameter(id: string, value: number): Promise<unknown> {
  return invoke("setParameter", id, value);
}

export type EnvGenCallbacks = {
  updateParams?: (id: string, value: number) => void;
};

export function setEnvGenCallbacks(callbacks: EnvGenCallbacks): void {
  (window as Window).__ENVGEN__ = {
    ...(window as Window).__ENVGEN__,
    ...callbacks,
  };
}

export function setUpdateParamsCallback(fn: (id: string, value: number) => void): void {
  setEnvGenCallbacks({ updateParams: fn });
}

import { useCallback, useEffect, useRef, useState } from "react";
import type { RefObject } from "react";
import type { MainModule } from "../types/wasmmodule";
import { createBridge } from "./bridge";
import { StepController, StepInterrupted } from "./StepController";
import type { PyodideInterface } from "./pyodideTypes";
import type { ConsoleLine, RunMode } from "./types";

const PYODIDE_INDEX_URL = "https://cdn.jsdelivr.net/pyodide/v314.0.3/full/";
const HIDDEN_API_URL = `${import.meta.env.BASE_URL}pyapi/algoplex_api.py`;

const AlgorithmState = { Idle: 0, Stepping: 1, Running: 2, Done: 3 } as const;

// both the Pyodide runtime and the API source are fetched/evaluated once and reused across runs (and across every instance of this hook),
// rather than per Run click.
let pyodidePromise: Promise<PyodideInterface> | null = null;
let hiddenApiSourcePromise: Promise<string> | null = null;

function loadPyodideRuntime(): Promise<PyodideInterface> {
  if (pyodidePromise) return pyodidePromise;

  pyodidePromise = (async () => {
    if (!window.loadPyodide) {
      await loadScript(`${PYODIDE_INDEX_URL}pyodide.js`);
    }
    if (!window.loadPyodide) {
      throw new Error(
        "Python failed to load from CDN — check your network settings.",
      );
    }
    const pyodide = await window.loadPyodide({ indexURL: PYODIDE_INDEX_URL });
    return pyodide;
  })();

  return pyodidePromise;
}

function loadHiddenApiSource(): Promise<string> {
  if (!hiddenApiSourcePromise) {
    hiddenApiSourcePromise = fetch(HIDDEN_API_URL).then((res) => {
      if (!res.ok) throw new Error(`Failed to fetch ${HIDDEN_API_URL}`);
      return res.text();
    });
  }
  return hiddenApiSourcePromise;
}

function loadScript(src: string): Promise<void> {
  return new Promise((resolve, reject) => {
    const script = document.createElement("script");
    script.src = src;
    script.onload = () => resolve();
    script.onerror = () => reject(new Error(`Failed to load ${src}`));
    document.head.appendChild(script);
  });
}

async function captureGlobalNames(pyodide: PyodideInterface): Promise<Set<string>> {
  const namesProxy = (await pyodide.runPythonAsync(
    "list(globals().keys())",
  )) as { toJs?: () => string[] } | string[];
  const names = Array.isArray(namesProxy) ? namesProxy : namesProxy.toJs?.() ?? [];
  return new Set(names);
}

//this wipes everything back to the hidden-API baseline before each run so every "Run"/"Step" starts clean with no stale values.
async function resetUserGlobals(
  pyodide: PyodideInterface,
  baseline: Set<string> | null,
): Promise<void> {
  if (!baseline) return;
  //capture the passed in/default baseline global values
  const baselineLiteral = JSON.stringify(Array.from(baseline));

  //using the set check for the globals in current run, if it is not in _algoplex_baseline, remove it from the global scope.
  await pyodide.runPythonAsync(
    `_algoplex_baseline = set(${baselineLiteral}) | {"_algoplex_baseline", "_algoplex_k"}\n` +
    `for _algoplex_k in list(globals().keys()):\n` +
    `    if _algoplex_k not in _algoplex_baseline:\n` +
    `        del globals()[_algoplex_k]\n` +
    `del _algoplex_baseline, _algoplex_k\n`,
  );
}

export function usePyRunner(moduleRef: RefObject<MainModule | null>) {
  const [status, setStatus] = useState<RunMode>("idle");
  const [consoleLines, setConsoleLines] = useState<ConsoleLine[]>([]);
  const [pyodideReady, setPyodideReady] = useState(false);

  const stepControllerRef = useRef(new StepController());
  const lineIdRef = useRef(0);
  const runTokenRef = useRef(0);
  const baselineGlobalsRef = useRef<Set<string> | null>(null);

  const pushLine = useCallback(
    (stream: ConsoleLine["stream"], text: string) => {
      if (!text) return;
      setConsoleLines((prev) => [
        ...prev,
        { id: lineIdRef.current++, stream, text },
      ]);
    },
    [],
  );

  // Warm up Pyodide + the hidden API as soon as the wasm module is ready,
  // so clicking "Run" the first time doesn't stall on a multi-MB download.
  useEffect(() => {
    let cancelled = false;
    (async () => {
      try {
        const pyodide = await loadPyodideRuntime();
        pyodide.setStdout({ batched: (msg) => pushLine("stdout", msg) });
        pyodide.setStderr({ batched: (msg) => pushLine("stderr", msg) });
        const apiSource = await loadHiddenApiSource();
        await pyodide.runPythonAsync(apiSource);
        baselineGlobalsRef.current = await captureGlobalNames(pyodide);
        if (!cancelled) setPyodideReady(true);
      } catch (err) {
        if (!cancelled) {
          pushLine("system", `Failed to initialize Python runtime: ${err}`);
        }
      }
    })();
    return () => {
      cancelled = true;
    };
  }, [pushLine]);

  useEffect(() => {
    const handler = (e: Event) => {
      const detail = (e as CustomEvent<{ stream: ConsoleLine["stream"]; text: string }>)
        .detail;
      pushLine(detail.stream, detail.text);
    };
    window.addEventListener("algoplex_console", handler);
    return () => window.removeEventListener("algoplex_console", handler);
  }, [pushLine]);

  const runEntry = useCallback(
    async (code: string, startMode: "running" | "paused", kind: "graph" | "bars") => {
      const module = moduleRef.current;
      if (!module) {
        pushLine("system", "Visualizer is still starting up — try again in a moment.");
        return;
      }
      // already executing
      if (status === "running" || status === "paused") return;

      const myToken = ++runTokenRef.current;
      const stepController = stepControllerRef.current;
      stepController.reset();
      if (startMode === "running") stepController.run();
      else stepController.step(); // arm exactly one pending step

      setConsoleLines([]);
      setStatus(startMode);
      // clear stale visited/discovered marks, stack/queue, and active-node only
      module._reset_run_state();
      module._set_algo_state(AlgorithmState.Running);

      try {
        const pyodide = await loadPyodideRuntime();
        if (!pyodideReady) {
          await loadHiddenApiSource().then((src) => pyodide.runPythonAsync(src));
          baselineGlobalsRef.current = await captureGlobalNames(pyodide);
        }
        if (myToken !== runTokenRef.current) return; // superseded by a later run

        await resetUserGlobals(pyodide, baselineGlobalsRef.current);

        const bridge = createBridge(() => moduleRef.current!, stepController);
        pyodide.globals.set("_bridge", bridge);

        const factoryName = kind === "bars" ? "_make_bars" : "_make_graph";
        const makeEntryArg = pyodide.globals.get(factoryName);
        if (!makeEntryArg)
          throw new Error(`API failed to load (${factoryName} missing).`);
        const entryArg = (makeEntryArg as (b: unknown) => { destroy?: () => void })(bridge);

        try {
          await pyodide.runPythonAsync(code);
          if (myToken !== runTokenRef.current) return;

          const mainFn = pyodide.globals.get("main");
          if (!mainFn) {
            throw new Error(
              'Your code needs an entry point: `async def main(graph):` ' +
              '(or `async def main(bars):` for a sorting scene).',
            );
          }

          await (mainFn as (g: unknown) => Promise<unknown>)(entryArg);
          if (myToken !== runTokenRef.current) return;

          module._set_algo_state(AlgorithmState.Done);
          setStatus("done");
          pushLine("system", "Finished.");
        } finally {
          entryArg.destroy?.();
        }
      } catch (err) {
        if (myToken !== runTokenRef.current) return;
        if (err instanceof StepInterrupted) {
          pushLine("system", "Stopped.");
        } else {
          module._set_algo_state(AlgorithmState.Idle);
          pushLine("stderr", formatPyError(err));
        }
        setStatus("error");
      }
    },
    [moduleRef, pushLine, status, pyodideReady],
  );

  const run = useCallback(
    (code: string, kind: "graph" | "bars" = "graph") => runEntry(code, "running", kind),
    [runEntry],
  );

  const stepOnce = useCallback(
    (code: string, kind: "graph" | "bars" = "graph") => {
      if (status === "idle" || status === "done" || status === "error") {
        runEntry(code, "paused", kind);
      } else {
        stepControllerRef.current.step();
      }
    },
    [runEntry, status],
  );

  const pause = useCallback(() => {
    stepControllerRef.current.pause();
    setStatus("paused");
  }, []);

  const resume = useCallback(() => {
    stepControllerRef.current.run();
    setStatus("running");
  }, []);

  const stop = useCallback(() => {
    runTokenRef.current++; // orphan any in-flight run
    stepControllerRef.current.interrupt();
    moduleRef.current?._reset_run_state();
    setStatus("idle");
  }, [moduleRef]);

  const clearConsole = useCallback(() => setConsoleLines([]), []);

  const setSpeed = useCallback((ms: number) => {
    stepControllerRef.current.setSpeed(ms);
  }, []);

  return {
    status,
    consoleLines,
    pyodideReady,
    run,
    stepOnce,
    pause,
    resume,
    stop,
    clearConsole,
    setSpeed,
  };
}

function formatPyError(err: unknown): string {
  if (err instanceof Error) {
    // Pyodide's PythonError.message is already a full Python traceback.
    return err.message;
  }
  return String(err);
}

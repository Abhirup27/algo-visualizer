import {
  useCallback,
  useEffect,
  useMemo,
  useRef,
  useState,
  type RefObject,
} from "react";
import { saveAs } from "file-saver";
import AlgoVisualizer from "./wasm/algo-visualizer.js";
import Navbar from "./components/navbar/Navbar.js";
import AlgoMenuPanel, {
  LOAD_ALGORITHM_EVENT,
  type AlgoDescriptor,
} from "./components/algo-menu-panel/AlgoMenuPanel.js";
import SettingsPanel from "./components/settings-panel/SettingsPanel.js";
import type { MainModule } from "./types/wasmmodule.d.ts";
import InfoPanel from "./components/info-panel/InfoPanel.tsx";
import CodePanel from "./components/code-panel/CodePanel.tsx";
import Tooltip, { type TooltipPage } from "./components/tooltip/Tooltip.tsx";
import StackView from "./components/stackView/StackView.tsx";
import { createTooltipPages } from "./tooltips.ts";
import type { GenericScriptStackFrame } from "./types/InfoPanel.ts";
import AdjacencyMatrix from "./components/adjMatrix/AdjacencyMatrix.tsx";
import { NodeDataInputHandler } from "./components/nodeDataInputModal/NodeDataInputHandler.tsx";
import useWindowSize from "./components/hooks/useWindowResize.ts";
import { usePyRunner } from "./pyodide/usePyRunner.ts";

const DEFAULT_ALGO_KEY = "menu";

function App() {
  const [currentStack, setStack] = useState<Array<GenericScriptStackFrame>>([]);
  //wasmModule
  const moduleRef: RefObject<MainModule | null> = useRef(null);

  const hasInitialized: RefObject<boolean> = useRef(false);
  const [showTooltip, setShowTooltip] = useState(true);
  const [showSidebar, setShowSidebar] = useState(true);

  const navbarRef: RefObject<HTMLElement> = useRef(null);
  const algoMenuPanelRef: RefObject<HTMLElement> = useRef(null);
  const settingsPanelRef: RefObject<HTMLElement> = useRef(null);
  const codePanelRef: RefObject<HTMLElement> = useRef(null);

  const infoPanel1Ref: RefObject<HTMLElement> = useRef(null);
  const infoPanel2Ref: RefObject<HTMLElement> = useRef(null);
  const canvasRef: RefObject<HTMLElement> = useRef(null);
  const [refsReady, setRefsReady] = useState(false);

  // Code being edited/run, and which algorithm it belongs to. Lifted up for the navbar's Run/Step/Pause/Stop controls
  const [code, setCode] = useState<string>("");
  const [activeAlgo, setActiveAlgo] = useState<AlgoDescriptor | null>(null);

  const {
    status,
    consoleLines,
    run,
    stepOnce,
    pause,
    resume,
    stop,
    clearConsole,
  } = usePyRunner(moduleRef);

  const loadAlgoFile = useCallback((key: string) => {
    const controller = new AbortController();
    fetch(`${import.meta.env.BASE_URL}algorithms/${key}.py`, {
      signal: controller.signal,
    })
      .then((response) => {
        if (!response.ok) throw new Error(`No sample file for "${key}"`);
        return response.text();
      })
      .then((data) => setCode(data))
      .catch((err) => {
        if (err.name !== "AbortError")
          console.error("Failed to load algorithm code:", err);
      });
    return controller;
  }, []);

  // Initial sample file, loaded once on mount.
  useEffect(() => {
    const controller = loadAlgoFile(DEFAULT_ALGO_KEY);
    return () => controller.abort();
  }, [loadAlgoFile]);

  // Sidebar-driven algorithm loading: AlgoMenuPanel dispatches this event
  // (with the algorithm's `key`, e.g. "bfs") whenever the user clicks an
  // entry in the algorithm list.
  useEffect(() => {
    const handler = (e: Event) => {
      const algo = (e as CustomEvent<AlgoDescriptor>).detail;
      stop();
      setActiveAlgo(algo);
      loadAlgoFile(algo.key);
    };
    window.addEventListener(LOAD_ALGORITHM_EVENT, handler);
    return () => window.removeEventListener(LOAD_ALGORITHM_EVENT, handler);
  }, [loadAlgoFile, stop]);

  // see include/scene_registry.hpp)
  const entryKind: "graph" | "bars" =
    activeAlgo?.category === "1" ? "bars" : "graph";

  useWindowSize(moduleRef, canvasRef);
  useEffect(() => {
    if (
      algoMenuPanelRef.current &&
      settingsPanelRef.current &&
      canvasRef.current &&
      infoPanel1Ref.current &&
      infoPanel2Ref.current &&
      codePanelRef.current &&
      navbarRef.current
    ) {
      setRefsReady(true);
    }
  }, []);
  const tooltipPages = useMemo(
    () =>
      createTooltipPages({
        algoMenuPanelRef,
        settingsPanelRef,
        navbarRef,
        canvasRef,
        infoPanel1Ref,
        infoPanel2Ref,
        codePanelRef,
      }),
    [],
  );

  function updateStack(stack: Array<GenericScriptStackFrame>) {
    setStack(stack);
  }

  useEffect(() => {
    if (hasInitialized.current) return;
    hasInitialized.current = true;

    AlgoVisualizer({
      canvas: (function () {
        var canvas = document.getElementById("canvas");

        // As a default initial behavior, pop up an alert when webgl context is lost.
        // To make your application robust, you may want to override this behavior before shipping!
        // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
        canvas.addEventListener(
          "webglcontextlost",
          function (e) {
            alert("WebGL context lost. You will need to reload the page.");
            e.preventDefault();
          },
          false,
        );

        return canvas;
      })(),

      print: (text: string) => console.log(text),
      printErr: (text: string) => console.error(text),
      onAbort: () => console.error("WASM aborted"),
      // setStatus: (text: string) => console.log(text),
      preRun: [],
      postRun: [],
      // This tells Emscripten's preload plugin to skip image decoding
      // Raylib loads images itself from the VFS — it doesn't need them pre-decoded
      // noImageDecodingc> true,
      // noAudioDecoding: true,
      locateFile: (path: string) => `${import.meta.env.BASE_URL}wasm/${path}`,
    }).then((module: MainModule) => {
      moduleRef.current = module;
    });
  }, []);

  return (
    <>
      <Navbar
        ref={navbarRef}
        status={status}
        onRun={() => run(code, entryKind)}
        onStep={() => stepOnce(code, entryKind)}
        onPause={pause}
        onResume={resume}
        onStop={stop}
      />
      <AlgoMenuPanel ref={algoMenuPanelRef} wasmModule={moduleRef} />
      <canvas
        id="canvas"
        ref={canvasRef}
        style={{ width: "100%", height: "100%" }}
        onContextMenu={(e) => e.preventDefault()}
        onMouseEnter={() => moduleRef.current?._set_receive_inputs(true)}
        onMouseLeave={() => moduleRef.current?._set_receive_inputs(false)}
      />
      {refsReady && showTooltip && (
        <Tooltip
          pages={tooltipPages}
          onClose={() => {
            setShowTooltip(false);
          }}
        />
      )}
      {canvasRef.current != null && (
        <NodeDataInputHandler wasmModule={moduleRef} canvasRef={canvasRef} />
      )}
      <SettingsPanel ref={settingsPanelRef} wasmModule={moduleRef!} />
      <InfoPanel ref={infoPanel1Ref} id="info1panel" type="Stack">
        <StackView
          items={currentStack}
          onUpdate={updateStack}
          wasmModule={moduleRef!}
        />
      </InfoPanel>
      <CodePanel
        ref={codePanelRef}
        code={code}
        onChangeCode={setCode}
        activeAlgo={activeAlgo}
        status={status}
        consoleLines={consoleLines}
        clearConsole={clearConsole}
      />
      <InfoPanel ref={infoPanel2Ref} id="info2panel" type="Graph">
        <AdjacencyMatrix wasmModule={moduleRef!} />
      </InfoPanel>
    </>
  );
}

export default App;

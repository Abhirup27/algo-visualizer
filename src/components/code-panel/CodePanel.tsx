import { lazy, Suspense } from "react";
import type { Ref } from "react";
import Console from "../console/Console.tsx";
import type { AlgoDescriptor } from "../algo-menu-panel/AlgoMenuPanel.tsx";
import type { ConsoleLine, RunMode } from "../../pyodide/types";

const MonacoEditor = lazy(() => import("../monacoEditor/MonacoEditor.tsx"));

export default function CodePanel({
  ref,
  code,
  onChangeCode,
  activeAlgo,
  status,
  consoleLines,
  clearConsole,
}: {
  ref: Ref<HTMLElement>;
  code: string;
  onChangeCode: (code: string) => void;
  activeAlgo: AlgoDescriptor | null;
  status: RunMode;
  consoleLines: ConsoleLine[];
  clearConsole: () => void;
}) {
  const isBusy = status === "running" || status === "paused";

  return (
    <div id="code-panel" ref={ref}>
      <div className="code-panel-toolbar">
        <span className="code-panel-title">
          {activeAlgo ? activeAlgo.name : "menu.py"}
        </span>

        <span className={`run-status run-status-${status}`}>
          {status}
        </span>
      </div>

      <div className="code-panel-content">
        <div className="code-panel-editor">
          <Suspense fallback={<h3>Loading...</h3>}>
            <MonacoEditor
              code={code}
              onChange={onChangeCode}
              readOnly={isBusy}
            />
          </Suspense>
        </div>

        <Console lines={consoleLines} onClear={clearConsole} />
      </div>
    </div>
  );
}

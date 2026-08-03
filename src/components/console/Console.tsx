import { useEffect, useRef } from "react";
import type { ConsoleLine } from "../../pyodide/types";

export default function Console({
  lines,
  onClear,
}: {
  lines: ConsoleLine[];
  onClear: () => void;
}) {
  const bottomRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    bottomRef.current?.scrollIntoView({ block: "end" });
  }, [lines.length]);

  return (
    <div className="console-panel">
      <div className="console-header">
        <span>Console</span>
        <button
          type="button"
          className="console-clear-btn"
          onClick={onClear}
          title="Clear console"
        >
          Clear
        </button>
      </div>
      <div className="console-output">
        {lines.length === 0 && (
          <div className="console-line console-line-system console-placeholder">
            Output from print() and runtime messages will show up here.
          </div>
        )}
        {lines.map((line) => (
          <div key={line.id} className={`console-line console-line-${line.stream}`}>
            {line.text}
          </div>
        ))}
        <div ref={bottomRef} />
      </div>
    </div>
  );
}

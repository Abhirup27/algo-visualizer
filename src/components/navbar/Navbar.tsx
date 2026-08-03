import { useState, type RefObject } from "react";
import type { RunMode } from "../../pyodide/types";

export default function Navbar({
  ref,
  status,
  onRun,
  onStep,
  onPause,
  onResume,
  onStop,
}: {
  ref: RefObject<HTMLElement>;
  status: RunMode;
  onRun: () => void;
  onStep: () => void;
  onPause: () => void;
  onResume: () => void;
  onStop: () => void;
}) {
  const [stepSpeed, setStepSpeed] = useState(1);
  const isBusy = status === "running" || status === "paused";

  return (
    <>
      <nav className="navbar" ref={ref}>
        <div className="navbar-title">
          <h3>AlgoPlex</h3>
        </div>
        <div className="navbar-actions">
          <button type="button" disabled={isBusy} onClick={onRun}>
            Start
          </button>
          <button
            type="button"
            disabled={status === "running"}
            onClick={onStep}
            title="Execute the next hidden-API call, then pause"
          >
            Step
          </button>
          {status === "paused" ? (
            <button type="button" onClick={onResume}>
              Resume
            </button>
          ) : (
            <button
              type="button"
              disabled={status !== "running"}
              onClick={onPause}
            >
              Pause
            </button>
          )}
          <button
            type="button"
            disabled={!isBusy}
            onClick={onStop}
            title="Stop and reset"
          >
            Stop
          </button>
          <span className={`run-status run-status-${status}`}>{status}</span>
        </div>
        <div className="navbar-speed-controls">
          <label htmlFor="step_speed">Algo speed</label>
          <div className="controls-row">
            <input
              type="range"
              name="step_speed"
              value={stepSpeed}
              min="0.5"
              max="10"
              step="0.5"
              onChange={(e) => {
                setStepSpeed(parseFloat(e.target.value));
              }}
            />
            <input
              type="number"
              name="step_speed_val"
              value={stepSpeed}
              min="0.5"
              max="10"
              onChange={(e) => {
                setStepSpeed(parseFloat(e.target.value));
              }}
            />
          </div>
        </div>
      </nav>
    </>
  );
}

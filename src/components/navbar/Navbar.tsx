import { useState, type RefObject } from "react";
import type { MainModule } from "../../types/wasmmodule";

export default function Navbar({
  ref,
  wasmModule,
}: {
  ref: RefObject<HTMLElement>;
  wasmModule: RefObject<MainModule>;
}) {
  const [stepSpeed, setStepSpeed] = useState(1);
  return (
    <>
      <nav className="navbar" ref={ref}>
        <div className="navbar-title">
          <h3>AlgoPlex</h3>
        </div>
        <div className="navbar-actions">
          <button
            type="button"
            onClick={(e) => {
              wasmModule.current._start_algo();
            }}
          >
            Start
          </button>
          <button
            type="button"
            onClick={(e) => {

              wasmModule.current._step_algo();
            }}
          >
            Step
          </button>
          <button type="button">Pause</button>
          <button type="button">Stop</button>
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


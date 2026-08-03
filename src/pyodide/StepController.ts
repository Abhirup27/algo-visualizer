// StepController is the mechanism that lets a user's arbitrary Python code
// be single-stepped from the UI, without workers, SharedArrayBuffer, or
// Atomics.wait.
//
// The trick: every hidden-API mutator (graph.stack.push, graph.queue.pop,
// graph.add_node, ...) is an `async def` that does
//
//     await bridge.wait_for_step()
//     <then the actual C++ call>
//
// Pyodide transparently awaits JS promises returned from a JsProxy call, so
// `wait_for_step()` genuinely suspends the Python coroutine — the browser
// stays responsive because nothing is blocked, the coroutine is just not
// scheduled again until the returned promise resolves.
//
// - In "running" mode, waitForStep() resolves after a delay (the step
//   speed), giving a smooth auto-play.
// - In "paused" mode, waitForStep() doesn't resolve until step() is called.
// - Clicking "Step" before the coroutine has even reached its first gate is
//   handled by a `pendingSteps` counter, so no click is ever lost to a race
//   between "user clicked Step" and "coroutine reached the await".

export type StepMode = "idle" | "running" | "paused";

export class StepController {
  private mode: StepMode = "idle";
  private pendingSteps = 0;
  private gate: (() => void) | null = null;
  private speedMs = 400;
  private interrupted = false;

  getMode(): StepMode {
    return this.mode;
  }

  setSpeed(ms: number) {
    this.speedMs = Math.max(0, ms);
  }

  /** Called by every hidden-API mutator before it touches the C++ scene. */
  async waitForStep(): Promise<void> {
    if (this.interrupted) throw new StepInterrupted();

    if (this.mode === "running") {
      await delay(this.speedMs);
      if (this.interrupted) throw new StepInterrupted();
      return;
    }

    if (this.pendingSteps > 0) {
      this.pendingSteps--;
      return;
    }

    await new Promise<void>((resolve) => {
      this.gate = resolve;
    });
    if (this.interrupted) throw new StepInterrupted();
  }

  /** Advance exactly one hidden-API call, then pause again. */
  step() {
    this.mode = "paused";
    if (this.gate) {
      const resolve = this.gate;
      this.gate = null;
      resolve();
    } else {
      this.pendingSteps++;
    }
  }

  /** Switch to continuous auto-play. */
  run() {
    this.mode = "running";
    if (this.gate) {
      const resolve = this.gate;
      this.gate = null;
      resolve();
    }
  }

  /** Freeze at the next hidden-API call boundary. */
  pause() {
    this.mode = "paused";
  }

  /** Hard-stop: any pending/future waitForStep() throws StepInterrupted. */
  interrupt() {
    this.interrupted = true;
    if (this.gate) {
      const resolve = this.gate;
      this.gate = null;
      resolve();
    }
  }

  /** Back to a clean slate for the next run. */
  reset() {
    this.mode = "idle";
    this.pendingSteps = 0;
    this.gate = null;
    this.interrupted = false;
  }
}

/** Thrown out of waitForStep() to unwind a running script on Reset/Stop. */
export class StepInterrupted extends Error {
  constructor() {
    super("Execution stopped by user");
    this.name = "StepInterrupted";
  }
}

function delay(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

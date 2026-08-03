// minimal typings for the subset of the Pyodide JS API this project uses.
// Pyodide is loaded from a CDN

export interface PyProxy {
  (...args: unknown[]): unknown;
  toJs?: (opts?: Record<string, unknown>) => unknown;
  destroy?: () => void;
}

export interface PyodideInterface {
  globals: {
    set: (name: string, value: unknown) => void;
    get: (name: string) => PyProxy | undefined;
  };
  runPython: (code: string) => unknown;
  runPythonAsync: (code: string) => Promise<unknown>;
  setStdout: (options: { batched?: (msg: string) => void }) => void;
  setStderr: (options: { batched?: (msg: string) => void }) => void;
}

declare global {
  interface Window {
    loadPyodide?: (config?: { indexURL?: string }) => Promise<PyodideInterface>;
  }
}

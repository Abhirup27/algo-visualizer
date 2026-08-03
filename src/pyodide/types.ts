// shared types for the Pyodide-based scripted execution runner.

export type RunMode = "idle" | "running" | "paused" | "done" | "error";

export interface ConsoleLine {
  id: number;
  stream: "stdout" | "stderr" | "system";
  text: string;
}

export interface AlgoDescriptor {
  id: string;
  key: string;
  name: string;
  category: string;
  subCategories: string;
}

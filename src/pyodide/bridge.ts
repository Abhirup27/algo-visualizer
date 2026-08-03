import type { MainModule } from "../types/wasmmodule";
import type { StepController } from "./StepController";

// This object is handed to Pyodide as a plain JS value
// (`pyodide.globals.set("_bridge", bridge)`), which Pyodide auto-wraps in a
// JsProxy. Every method below becomes directly callable from Python as
// `_bridge.method_name(...)`, and — because Pyodide awaits thenables
// automatically — an `async` method here can be `await`ed from an `async
// def` in Python. This is the literal mechanism by which
// `await graph.stack.push(node_id)` in a user's script ends up calling a
// specific compiled C++ function through JS.
//
// Every mutator here does the *actual* C++ call — the "should I pause
// first" decision (`wait_for_step`) is a separate bridge method that the
// Python hidden API always awaits first, so this file has no stepping logic
// of its own; it is a pure, dumb translation layer.

export interface AlgoplexBridge {
  wait_for_step(): Promise<void>;

  initial_nodes_json(): string;
  initial_adjacency_json(): string;
  initial_root_json(): string;

  push_stack(nodeId: number): Promise<number>;
  pop_stack(): Promise<number>;
  stack_size(): number;

  enqueue(nodeId: number): Promise<void>;
  dequeue(): Promise<number>;
  queue_size(): number;

  mark_visited(nodeId: number, visited: boolean): Promise<void>;
  mark_discovered(nodeId: number, discovered: boolean): Promise<void>;
  set_current(nodeId: number): Promise<void>;

  add_node(data: number): Promise<number>;
  add_edge(fromId: number, toId: number): Promise<void>;
  remove_node(nodeId: number): Promise<void>;
  remove_edge(fromId: number, toId: number): Promise<void>;

  // Stubbed: no C++ BarScene exists yet see algoplex_api.py -> class Bars
  swap_bars(i: number, j: number): Promise<void>;
  compare_bars(i: number, j: number): Promise<number>;
  set_bar(i: number, value: number): Promise<void>;
  bar_count(): number;

  log(message: string): void;
}

export function createBridge(
  getModule: () => MainModule,
  stepController: StepController,
): AlgoplexBridge {
  const mod = () => getModule();

  return {
    wait_for_step: () => stepController.waitForStep(),

    initial_nodes_json: () => mod().UTF8ToString(mod()._get_node_list_json()),
    initial_adjacency_json: () => mod().UTF8ToString(mod()._get_adj_json()),
    initial_root_json: () => mod().UTF8ToString(mod()._get_root_node_json()),

    push_stack: async (nodeId: number) => mod()._script_stack_push(nodeId),
    pop_stack: async () => mod()._script_stack_pop(),
    stack_size: () => mod()._script_stack_size(),

    enqueue: async (nodeId: number) => {
      mod()._script_queue_enqueue(nodeId);
    },
    dequeue: async () => mod()._script_queue_dequeue(),
    queue_size: () => mod()._script_queue_size(),

    mark_visited: async (nodeId: number, visited: boolean) => {
      mod()._mark_visited(nodeId, visited ? 1 : 0);
    },
    mark_discovered: async (nodeId: number, discovered: boolean) => {
      mod()._mark_discovered(nodeId, discovered ? 1 : 0);
    },
    set_current: async (nodeId: number) => {
      mod()._set_active_node(nodeId);
    },

    add_node: async (data: number) => mod()._add_node(data),
    add_edge: async (fromId: number, toId: number) => {
      mod()._add_edge(fromId, toId);
    },
    remove_node: async (nodeId: number) => {
      mod()._remove_node(nodeId);
    },
    remove_edge: async (fromId: number, toId: number) => {
      mod()._remove_edge(fromId, toId);
    },

    swap_bars: async () => {
      throw new Error(
        "Bars scene isn't implemented in C++ yet (see scene.cpp's " +
        "script_stack_push/pop for the pattern to follow for swap_bars).",
      );
    },
    compare_bars: async () => {
      throw new Error("Bars scene isn't implemented in C++ yet.");
    },
    set_bar: async () => {
      throw new Error("Bars scene isn't implemented in C++ yet.");
    },
    bar_count: () => 0,

    // Lets the hidden API surface Python-side warnings
    log: (message: string) => {
      window.dispatchEvent(
        new CustomEvent("algoplex_console", {
          detail: { stream: "system", text: message },
        }),
      );
    },
  };
}

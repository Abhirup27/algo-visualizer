// TypeScript bindings for emscripten-generated code.  Automatically generated at compile time.
declare namespace RuntimeExports {
  /**
   * @param {string|null=} returnType
   * @param {Array=} argTypes
   * @param {Array=} args
   * @param {Object=} opts
   */
  function ccall(ident: any, returnType?: (string | null) | undefined, argTypes?: any[] | undefined, args?: any[] | undefined, opts?: any | undefined): any;
  /**
   * @param {string=} returnType
   * @param {Array=} argTypes
   * @param {Object=} opts
   */
  function cwrap(ident: any, returnType?: string | undefined, argTypes?: any[] | undefined, opts?: any | undefined): any;
  /**
   * Given a pointer 'ptr' to a null-terminated UTF8-encoded string in the
   * emscripten HEAP, returns a copy of that string as a Javascript String object.
   *
   * @param {number} ptr
   * @param {number=} maxBytesToRead - An optional length that specifies the
   *   maximum number of bytes to read. You can omit this parameter to scan the
   *   string until the first 0 byte. If maxBytesToRead is passed, and the string
   *   at [ptr, ptr+maxBytesToReadr[ contains a null byte in the middle, then the
   *   string will cut short at that byte index.
   * @param {boolean=} ignoreNul - If true, the function will not stop on a NUL character.
   * @return {string}
   */
  function UTF8ToString(ptr: number, maxBytesToRead?: number | undefined, ignoreNul?: boolean | undefined): string;
  function FS_createPath(...args: any[]): any;
  function FS_createDataFile(...args: any[]): any;
  function FS_preloadFile(parent: any, name: any, url: any, canRead: any, canWrite: any, dontCreateFile: any, canOwn: any, preFinish: any): Promise<void>;
  function FS_unlink(...args: any[]): any;
  function FS_createLazyFile(...args: any[]): any;
  function FS_createDevice(...args: any[]): any;
  function addRunDependency(id: any): void;
  function removeRunDependency(id: any): void;
}
interface WasmModule {
  _set_receive_inputs(_0: number): void;
  _update_mode(_0: number, _1: number, _2: number): void;
  _get_scene_ptr(): number;
  _on_resize(): void;
  _get_current_algorithm_id(): number;
  _get_adj_json(): number;
  _get_node_list_json(): number;
  _get_root_node_json(): number;
  _reset_scene(): void;
  _reset_run_state(): void;
  _toggle_keybind_overlay(): void;
  _set_root_node(_0: number): void;
  _set_node_val(_0: number, _1: number): void;
  _save_camera_pos(): void;
  _set_camera_pos_to_old_pos(): void;
  _set_hover_state(_0: number, _1: number): void;
  _set_algorithm(_0: number): void;
  _script_stack_push(_0: number): number;
  _script_stack_pop(): number;
  _script_stack_size(): number;
  _script_queue_enqueue(_0: number): void;
  _script_queue_dequeue(): number;
  _script_queue_size(): number;
  _mark_visited(_0: number, _1: number): void;
  _mark_discovered(_0: number, _1: number): void;
  _set_active_node(_0: number): void;
  _add_node(_0: number): number;
  _add_edge(_0: number, _1: number): void;
  _remove_node(_0: number): void;
  _remove_edge(_0: number, _1: number): void;
  _set_algo_state(_0: number): void;
  _get_script_stack_json(): number;
  _get_script_queue_json(): number;
  _main(_0: number, _1: number): number;
}

export type MainModule = WasmModule & typeof RuntimeExports;
export default function MainModuleFactory(options?: unknown): Promise<MainModule>;

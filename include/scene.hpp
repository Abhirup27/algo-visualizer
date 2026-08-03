#pragma once
#include "algorithm.hpp"
#include "app_state.hpp"
#include "arena.hpp"
#include "raylib.h"
#include "scene_registry.hpp"
#include "utils.hpp"
#include <cstddef>
#include <cstdint>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/em_macros.h>
#include <emscripten/em_types.h>
#include <stdint.h>
class Scene {
  AppState state;

public:
  Arena m_parentArena;

  // this is set by set_algo_state() as a Pyodide run starts/steps/finishes.
  AlgorithmState algorithm_state = AlgorithmState::Idle;

  SceneType m_SceneType;
  RenderTexture2D target;
  Font *m_font;
  Camera2D g_camera;
  bool m_updateRes = false;
  bool m_showUI = true;
  Vector2 camera_old_offset;
  float camera_old_zoom;
  Vector2 camera_old_target;
  Vector2 mouse_world_pos;

  Scene();
  Scene(Font *, Arena);
  virtual ~Scene();
  virtual void init() = 0;
  virtual void draw(IVector2 *) = 0;
  virtual void update(IVector2 *) = 0;
  virtual void input() {};

  virtual void switchAlgorithm(AlgorithmId) = 0;
  // JS bridge functions

  virtual void resetScene() = 0;
  // resets everything else other than the actual node/edge structure the user placed on the canvas, this is called by pyodide module when the user starts a script
  virtual void resetRunState() {}
  virtual void onResize();
  virtual void updateMode(int, int) = 0;
  virtual void setHoverState(bool, uint32_t) = 0;
  virtual void toggleUI();
  void saveCameraPos();
  virtual void resetCameraPos() = 0;

  virtual const char *getAdjJSON();
  virtual const char *getNodeListJSON();
  virtual const char *getRootNodeJSON();

  virtual void setRootNode(uint32_t);
  virtual void setNodeVal(uint32_t, int);

  // --- Python API surface --------------------------------------
  // Each of these is the C++-side handler for exactly one API call a user's Pyodide-driven script can make (see public/pyapi/algoplex_api.py and src/pyodide/bridge.ts).
  // Declared here so the extern "C" bridge below can dispatch through App::instance->current_scene the same way getAdjJSON()/setRootNode()
  //sub class define behavior;
  virtual uint32_t scriptStackPush(uint32_t node_id);
  virtual uint32_t scriptStackPop();
  virtual size_t scriptStackSize();

  virtual void scriptQueueEnqueue(uint32_t node_id);
  virtual uint32_t scriptQueueDequeue();
  virtual size_t scriptQueueSize();

  virtual void markVisited(uint32_t node_id, bool visited_flag);
  virtual void markDiscovered(uint32_t node_id, bool discovered_flag);
  virtual void setActiveNode(uint32_t node_id);

  virtual uint32_t addNode(int64_t data);
  virtual void addEdge(uint32_t from_id, uint32_t to_id);
  virtual void removeNode(uint32_t node_id);
  virtual void removeEdge(uint32_t from_id, uint32_t to_id);

  virtual void setAlgoState(AlgorithmState state);

  virtual const char *getScriptStackJSON();
  virtual const char *getScriptQueueJSON();
};

extern "C" {
void EMSCRIPTEN_KEEPALIVE reset_scene();
void EMSCRIPTEN_KEEPALIVE reset_run_state();
void EMSCRIPTEN_KEEPALIVE toggle_keybind_overlay();
void EMSCRIPTEN_KEEPALIVE update_mode(int, int);
void EMSCRIPTEN_KEEPALIVE on_resize();
void EMSCRIPTEN_KEEPALIVE set_root_node(uint32_t);
void EMSCRIPTEN_KEEPALIVE set_node_val(uint32_t, int);
void EMSCRIPTEN_KEEPALIVE set_hover_state(bool, uint32_t);
void EMSCRIPTEN_KEEPALIVE save_camera_pos();
void EMSCRIPTEN_KEEPALIVE set_camera_pos_to_old_pos();

const char *EMSCRIPTEN_KEEPALIVE get_adj_json();
const char *EMSCRIPTEN_KEEPALIVE get_node_list_json();
const char *EMSCRIPTEN_KEEPALIVE get_root_node_json();

// --- Python API exports -----
// One-to-one with the JS bridge in src/pyodide/bridge.ts, which is in turn is used by Python API classes (Stack/Queue/Graph) that
// wrap every call the user's script makes with an await wait_for_step().
uint32_t EMSCRIPTEN_KEEPALIVE script_stack_push(uint32_t node_id);
uint32_t EMSCRIPTEN_KEEPALIVE script_stack_pop();
uint32_t EMSCRIPTEN_KEEPALIVE script_stack_size();

void EMSCRIPTEN_KEEPALIVE script_queue_enqueue(uint32_t node_id);
uint32_t EMSCRIPTEN_KEEPALIVE script_queue_dequeue();
uint32_t EMSCRIPTEN_KEEPALIVE script_queue_size();

void EMSCRIPTEN_KEEPALIVE mark_visited(uint32_t node_id, int flag);
void EMSCRIPTEN_KEEPALIVE mark_discovered(uint32_t node_id, int flag);
void EMSCRIPTEN_KEEPALIVE set_active_node(uint32_t node_id);

uint32_t EMSCRIPTEN_KEEPALIVE add_node(int64_t data);
void EMSCRIPTEN_KEEPALIVE add_edge(uint32_t from_id, uint32_t to_id);
void EMSCRIPTEN_KEEPALIVE remove_node(uint32_t node_id);
void EMSCRIPTEN_KEEPALIVE remove_edge(uint32_t from_id, uint32_t to_id);

void EMSCRIPTEN_KEEPALIVE set_algo_state(int state);

const char *EMSCRIPTEN_KEEPALIVE get_script_stack_json();
const char *EMSCRIPTEN_KEEPALIVE get_script_queue_json();
}

#pragma once
#include "algorithm.hpp"
#include "app_state.hpp"
#include "arena.hpp"
#include "raylib.h"
#include "scene_registry.hpp"
#include "utils.hpp"
#include <cstdint>
#import <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/em_macros.h>
#import <emscripten/em_types.h>
#include <stdint.h>
class Scene {
  AppState state;

public:
  Arena m_parentArena;
  Arena m_algoArena;

  IAlgorithm *m_algorithmInstance;

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
  ~Scene();
  virtual void init() = 0;
  virtual void draw(IVector2 *) = 0;
  virtual void update(IVector2 *) = 0;
  virtual void input() {};

  virtual void createAlgorithmInstance(AlgorithmId) = 0;
  virtual void switchAlgorithm(AlgorithmId) = 0;
  // JS bridge functions

  virtual void resetScene() = 0;
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
};

extern "C" {
void EMSCRIPTEN_KEEPALIVE reset_scene();
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

void start_algo();
void step_algo();
}

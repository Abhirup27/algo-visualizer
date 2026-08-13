#include "scene.hpp"
#include "app.hpp"
#include "colors.h"
#include "raylib.h"
#include "scene.hpp"
#include "scene_registry.hpp"
#include <cstdint>
#include <cstdio>

Scene::Scene()
    : m_SceneType(SceneType::Graph), g_camera({{0}}), m_updateRes(false) {}

Scene::~Scene() {}

Scene::Scene(Font *font, AlgorithmId id, Arena parentArena)
    : m_parentArena(parentArena), m_algoId(id), m_SceneType(SceneType::Graph),
      m_font(font), g_camera({{0}}), m_showUI(true) {}

void Scene::init() {

  IVector2 *resolution = App::m_GetInstance().m_GetResolution();
  g_camera.target = {0, 0};
  g_camera.offset = {resolution->x * 0.5f, resolution->y * 0.5f};
  g_camera.zoom = 1.5f;

  m_SceneType = SceneType::Graph;
}

void Scene::draw(IVector2 *resolution) {
  char text[64];

  snprintf(text, 64, "Select an algorithm in the sidebar.");
  Vector2 strWidth = MeasureTextEx(*m_font, text, 25, 2);
  DrawTextEx(
      *m_font, text,
      {(resolution->x - strWidth.x) / 2, static_cast<float>(resolution->y / 2)},
      25, 2, COLOR_TEXT);
}

void Scene::update(IVector2 *resolution) {
  if (m_updateRes) {

    // What world point is currently at the center of the OLD canvas?
    Vector2 old_center_world = GetScreenToWorld2D(
        {resolution->x * 0.5f, resolution->y * 0.5f}, g_camera);

    double css_w, css_h;
    emscripten_get_element_css_size("#canvas", &css_w, &css_h);
    double dpr = emscripten_get_device_pixel_ratio();
    *resolution = {static_cast<int>(std::ceil(css_w * dpr)),
                   static_cast<int>(std::ceil(css_h * dpr))};
    SetWindowSize(resolution->x, resolution->y);

    g_camera.offset = {resolution->x * 0.5f, resolution->y * 0.5f};
    g_camera.target = old_center_world; // keep that same world point centered

    m_updateRes = false;
  }
}

void Scene::toggleUI() { m_showUI = !m_showUI; }

void Scene::saveCameraPos() {

  camera_old_offset = g_camera.offset;

  camera_old_target = g_camera.target;
  camera_old_zoom = g_camera.zoom;
}

void Scene::onResize() { m_updateRes = true; }

void Scene::resetCameraPos() {}

uint32_t Scene::getCurrentAlgorithmId() {
  return static_cast<uint32_t>(this->m_algoId);
}
const char *Scene::getAdjJSON() { return ""; }
const char *Scene::getNodeListJSON() { return ""; }
const char *Scene::getRootNodeJSON() { return ""; }
void Scene::setRootNode(uint32_t node_id) {}
void Scene::setNodeVal(uint32_t node_id, int value) {}

uint32_t Scene::scriptStackPush(uint32_t node_id) { return UINT32_MAX; }
uint32_t Scene::scriptStackPop() { return UINT32_MAX; }
size_t Scene::scriptStackSize() { return 0; }

void Scene::scriptQueueEnqueue(uint32_t node_id) {}
uint32_t Scene::scriptQueueDequeue() { return UINT32_MAX; }
size_t Scene::scriptQueueSize() { return 0; }

void Scene::markVisited(uint32_t node_id, bool visited_flag) {}
void Scene::markDiscovered(uint32_t node_id, bool discovered_flag) {}
void Scene::setActiveNode(uint32_t node_id) {}

uint32_t Scene::addNode(int64_t data) { return UINT32_MAX; }
void Scene::addEdge(uint32_t from_id, uint32_t to_id) {}
void Scene::removeNode(uint32_t node_id) {}
void Scene::removeEdge(uint32_t from_id, uint32_t to_id) {}

void Scene::setAlgoState(AlgorithmState state) {}

const char *Scene::getScriptStackJSON() { return "[]"; }
const char *Scene::getScriptQueueJSON() { return "[]"; }
extern "C" {
void on_resize(void) { App::instance->current_scene->onResize(); }

void set_root_node(uint32_t idx) {

  App::instance->current_scene->setRootNode(idx);
}

void set_node_val(uint32_t node_id, int value) {
  App::instance->current_scene->setNodeVal(node_id, value);
}

uint32_t get_current_algorithm_id() {
  return App::instance->current_scene->getCurrentAlgorithmId();
}
const char *get_adj_json() {
  return App::instance->current_scene->getAdjJSON();
}
const char *get_node_list_json() {
  return App::instance->current_scene->getNodeListJSON();
}
const char *get_root_node_json() {
  return App::instance->current_scene->getRootNodeJSON();
}

void reset_scene() { return App::instance->current_scene->resetScene(); }

void reset_run_state() { return App::instance->current_scene->resetRunState(); }

void toggle_keybind_overlay() { App::instance->current_scene->toggleUI(); }

void save_camera_pos() { App::instance->current_scene->saveCameraPos(); }

void set_camera_pos_to_old_pos() {

  App::instance->current_scene->resetCameraPos();
}

void set_hover_state(bool hover, uint32_t node_id) {
  App::instance->current_scene->setHoverState(hover, node_id);
}
void update_mode(int primary, int secondary) {
  App::instance->current_scene->updateMode(primary, secondary);
}

// --- Python API exports ---------------------------------------------
// One-to-one with the JS bridge in src/pyodide/bridge.ts, which is in turn is
// one-to-one with the hidden Python API classes (Stack/Queue/Graph) that wrap every call the user's script makes with an `await wait_for_step()`.
uint32_t script_stack_push(uint32_t node_id) {
  return App::instance->current_scene->scriptStackPush(node_id);
}
uint32_t script_stack_pop() {
  return App::instance->current_scene->scriptStackPop();
}
uint32_t script_stack_size() {
  return static_cast<uint32_t>(App::instance->current_scene->scriptStackSize());
}

void script_queue_enqueue(uint32_t node_id) {
  App::instance->current_scene->scriptQueueEnqueue(node_id);
}
uint32_t script_queue_dequeue() {
  return App::instance->current_scene->scriptQueueDequeue();
}
uint32_t script_queue_size() {
  return static_cast<uint32_t>(App::instance->current_scene->scriptQueueSize());
}

void mark_visited(uint32_t node_id, int flag) {
  App::instance->current_scene->markVisited(node_id, flag != 0);
}
void mark_discovered(uint32_t node_id, int flag) {
  App::instance->current_scene->markDiscovered(node_id, flag != 0);
}
void set_active_node(uint32_t node_id) {
  App::instance->current_scene->setActiveNode(node_id);
}

uint32_t add_node(int64_t data) {
  return App::instance->current_scene->addNode(data);
}
void add_edge(uint32_t from_id, uint32_t to_id) {
  App::instance->current_scene->addEdge(from_id, to_id);
}
void remove_node(uint32_t node_id) {
  App::instance->current_scene->removeNode(node_id);
}
void remove_edge(uint32_t from_id, uint32_t to_id) {
  App::instance->current_scene->removeEdge(from_id, to_id);
}

void set_algo_state(int state) {
  App::instance->current_scene->setAlgoState(
      static_cast<AlgorithmState>(state));
}

const char *get_script_stack_json() {
  return App::instance->current_scene->getScriptStackJSON();
}
const char *get_script_queue_json() {
  return App::instance->current_scene->getScriptQueueJSON();
}
}

EMSCRIPTEN_BINDINGS(scene_bindings) {

  emscripten::class_<Scene>("Scene")
      .function("resetScene", &Scene::resetScene)
      .function("resetRunState", &Scene::resetRunState)
      .function("onResize", &Scene::onResize)
      .function("saveCameraPos", &Scene::saveCameraPos)
      .function("resetCameraPos", &Scene::resetCameraPos)
      .function("getAdjJSON", &Scene::getAdjJSON,
                emscripten::allow_raw_pointers())
      .function("getNodeListJSON", &Scene::getNodeListJSON,
                emscripten::allow_raw_pointers())
      .function("getRootNodeJSON", &Scene::getRootNodeJSON,
                emscripten::allow_raw_pointers())
      .function("setRootNode", &Scene::setRootNode)
      .function("setNodeVal", &Scene::setNodeVal)
      .function("setHoverState", &Scene::setHoverState)
      .function("toggleUI", &Scene::toggleUI)
      .function("updateMode", &Scene::updateMode);
};

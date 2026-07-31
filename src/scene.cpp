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
Scene::Scene(Font *font, Arena parentArena)
    : m_parentArena(parentArena), m_SceneType(SceneType::Graph), m_font(font),
      g_camera({{0}}), m_showUI(true) {}

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
const char *Scene::getAdjJSON() {}
const char *Scene::getNodeListJSON() {}
const char *Scene::getRootNodeJSON() {}
void Scene::setRootNode(uint32_t node_id) {}
void Scene::setNodeVal(uint32_t node_id, int value) {}
extern "C" {
void on_resize(void) { App::instance->current_scene->onResize(); }

void set_root_node(uint32_t idx) {

  App::instance->current_scene->setRootNode(idx);
}

void set_node_val(uint32_t node_id, int value) {
  App::instance->current_scene->setNodeVal(node_id, value);
}

// const char *get_stack_json() {
//   // return App::instance->current_scene->getStackJSON();
// }
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

void step_algo() {}
void start_algo() {}
}

EMSCRIPTEN_BINDINGS(scene_bindings) {

  emscripten::class_<Scene>("Scene")
      .function("resetScene", &Scene::resetScene)
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

#include "graph_scene.hpp"
#include "menu.hpp"
#include "raylib.h"
#include "scene.hpp"
#include "scene_registry.hpp"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <string>
#if defined(PLATFORM_WEB)
#include "web.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#include "app.hpp"
#include <memory>

App *App::instance = nullptr;

App::App(IVector2 resolution, const char *font)
    : m_resolution({.x = resolution.x, .y = resolution.y}),
      m_font(LoadFont(font)), m_mouse_hovering(true) {
  // createInstance(width, height);
}

App::~App() {
  void *buffer = this->m_arena.buffer;
  arena_free(&this->m_arena);
  // unload scene before freeing the memory

  free(buffer);

  //unload resources
  UnloadFont(this->m_font);

  emscripten_pause_main_loop();
  emscripten_cancel_main_loop();
  emscripten_exit_with_live_runtime();
  CloseWindow();
}

void initApp(App *app, IVector2 resolution, const char *font) {

  //the entire scene will be in this memory
  void *mem_buffer = malloc(uint64_t(10ULL * 1024 * 1024));
  arena_init(&app->m_arena, mem_buffer, 10ULL * 1024 * 1024);

  if (resolution.x == 0 && resolution.y == 0) {

#if defined(PLATFORM_WEB)
    double css_x, css_y;

    emscripten_get_element_css_size("#canvas", &css_x, &css_y);

    double dpr = emscripten_get_device_pixel_ratio();

    app->m_resolution = {.x = (int)(css_x * dpr), .y = (int)(css_y * dpr)};

    app->m_mouse_hovering = true;

#endif

  } else {

    app->m_resolution = resolution;
  }

  //Load default scene here.MENU is just an empty scene;
  app->m_app_state = AppState::MENU;
  App::instance = app;

  notify_algorithms();

  InitWindow(app->m_resolution.x, app->m_resolution.y, "AlgoPlex");
  SetExitKey(KEY_NULL);

  char path[256];
  snprintf(path, sizeof(path), "%sfont/%s", RESOURCES_PATH, font);
  app->m_font = LoadFont(path);
  if (IsWindowReady()) {
    //allocate memory to load empty scene;
    app->current_scene =
        arena_create<Menu>(&app->m_arena, &app->m_font, &app->m_arena);
    app->current_scene->init();
    // app->instance->run();
    notify_algorithms();
    emscripten_set_main_loop(app->m_RunWrapper, 60, 1);
  }
}

App &App::m_GetInstance() { return *instance; }

IVector2 *App::m_GetResolution() { return &m_resolution; }

// void App::setState(std::unique_ptr<State> state) {
//   current_state = std::move(state);
//   // current_state->init();
// }
// void App::setState(AppState new_state) { g_app_state = new_state; }

void App::m_Run(void) {
  if (App::m_app_state != QUIT) {
    if (App::m_mouse_hovering == true) {

      current_scene->input();
    }
    current_scene->update(&m_resolution);
    current_scene->draw(&m_resolution);
  } else {
    // this->setState(std::make_unique<Menu>("Algorithm Visualizer"));
    App::m_Shutdown();
  }
}
void App::m_Shutdown() {
  if (current_scene) {
    // current_scene->unload();
    // current_scene->~Scene();
  }
  UnloadFont(m_font);
  instance = nullptr;
  CloseWindow();
#if defined(PLATFORM_WEB)
  emscripten_pause_main_loop();
  emscripten_cancel_main_loop();
  emscripten_exit_with_live_runtime();
#endif
}

void App::m_RunWrapper() { App::instance->m_Run(); }

void App::m_LoadAlgorithm(int algorithm_id) {

  const AlgorithmInfo &info = ALGORITHMS[static_cast<int>(algorithm_id)];
  if (current_scene != nullptr) {
    //current_scene->m_unload();
    current_scene->Scene::~Scene();
  }
  arena_reset(&m_arena);

  switch (info.category) {
  case SceneType::Graph:

    current_scene = arena_create<GraphScene>(&m_arena, &m_font, m_arena);
    current_scene->init();
    break;

  case SceneType::Sort:

    break;

  default:

    break;
  }
}

#if defined(PLATFORM_WEB)
// void App::initWeb() {
//
//   // Need to return EventDescriptors from init that I will dispatch after I set
//   // the main loop, so that the wasm module has the required functions that will
//   // be used by react.
//   current_scene->init();
//
//   notify_algorithms();
//   emscripten_set_main_loop(m_RunWrapper, 60, 1);
// }
//
extern "C" void notify_algorithms() {
  std::string json = "[";

  for (uint16_t i = 0; i < ALGORITHM_COUNT; i++) {
    const auto &a = ALGORITHMS[i];

    if (i > 0)
      json += ",";
    json += "{";
    json += "\"id\":\"" +
            std::string(std::to_string(std::to_underlying(a.id))) + "\",";
    json += "\"name\":\"" + std::string(a.name) + "\",";
    json += "\"category\":\"" +
            std::string(std::to_string(std::to_underlying(a.category))) + "\",";
    json += "\"subCategories\":\"" + std::string(a.subCategories) + "\"";
    json += "}";
  }
  json += "]";
  js_register_algorithms(json.c_str());
}

extern "C" void set_receive_inputs(bool mouseHover) {
  App::m_GetInstance().m_mouse_hovering = mouseHover;
}

extern "C" void set_algorithm(int algorithm_id) {
  App::m_GetInstance().m_LoadAlgorithm(algorithm_id);
}
#endif

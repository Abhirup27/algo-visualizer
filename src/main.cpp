#include "raylib.h"
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <stdio.h>

/*
 * WASM imports and JS functions.
 * */
#if defined(PLATFORM_WEB)
#include <emscripten/console.h>
#include <emscripten/em_js.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

EM_JS(void, toggle_console, (), {
  console = document.getElementById("output");
  console.hidden = !console.hidden;
})
EM_JS(int, canvas_set_size, (), {
  canvas = document.getElementById("canvas");
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  return window.innerWidth * window.innerHeight;
})

EM_JS(void, print_alert, (int width), { alert(width); })
#endif

/**
 *  ImGUI imports
 * */
#pragma region imgui
#include "imgui.h"
#include "imguiThemes.h"
#include "rlImGui.h"
ImGuiIO *g_io = nullptr;
#pragma endregion

int width = 1280;
int height = 720;

void UpdateDrawFrame(void);

/**
 * States of the algorithm.
 * */
enum class AlgorithmState { Idle, Stepping, Running, Done };

AlgorithmState algorithmState = AlgorithmState::Idle;

std::queue<int> q;
std::vector<bool> visited;

struct Node_A {
  Vector2 pos;
  uint16_t radius;
  Rectangle collider;

  int64_t data;
};
struct Node_L {
  Vector2 pos;
  uint16_t radius;
  Rectangle collider;

  int64_t data;
  Node_L *edges;
};

Node_L *root = (Node_L *)malloc(sizeof(Node_L));

int main(void) {

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
#if defined(PLATFORM_WEB)
  printf("%d", canvas_set_size());

  emscripten_get_canvas_element_size("#canvas", &width, &height);
// printf("%d", width);
#endif

  InitWindow(width, height, "Algorithm Visualizer - raylib");

  root->pos = {static_cast<float>(width) / 2.0f,
               static_cast<float>(height) / 2.0f};
  root->radius = 10;
  root->collider = {width / 2.0f, height / 2.0f, 15, 15};

#pragma region imgui
  rlImGuiSetup(true);

  // you can use whatever imgui theme you like!
  // ImGui::StyleColorsDark();
  // imguiThemes::yellow();
  // imguiThemes::gray();
  imguiThemes::green();
  // imguiThemes::red();
  // imguiThemes::embraceTheDarkness();

  // ImGuiIO &io = ImGui::GetIO(); (void)io;

  g_io = &ImGui::GetIO();
  g_io->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  g_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
  g_io->FontGlobalScale = 2;

  ImGuiStyle &style = ImGui::GetStyle();
  if (g_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    // style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 0.5f;
    // style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
  }

#pragma endregion

#if defined(PLATFORM_WEB)
  emscripten_get_canvas_element_size("#canvas", &width, &height);

  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    UpdateDrawFrame();
  }
#endif

#pragma region imgui
  rlImGuiShutdown();
#pragma endregion

  CloseWindow();

  return 0;
}

void UpdateDrawFrame(void) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

#if defined(PLATFORM_WEB)
  emscripten_get_canvas_element_size("#canvas", &width, &height);
  // printf("%d", width);
  // print_alert(width);
#elif defined(PLATFORM_DESKTOP)
  width = GetScreenWidth();
  height = GetScreenHeight();
#endif
  DrawLineEx({0U, 0U}, {static_cast<float>(width), static_cast<float>(height)},
             3, RED);
  DrawCircleLines(root->pos.x, root->pos.y, root->radius, RED);

#pragma region imgui
  rlImGuiBegin();

  ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
  ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  ImGui::PopStyleColor(2);
#pragma endregion

  ImGui::Begin("Algorithm Visualizer");

  ImGui::Text("Settings");
#if defined(PLATFORM_WEB)
  if (ImGui::Button("Toggle Web Console")) {
    toggle_console();
  }
#endif
  if (algorithmState == AlgorithmState::Idle) {
    ImGui::Button("Add node");

    if (ImGui::Button("Start")) {
      algorithmState = AlgorithmState::Stepping;
      //  resetAlgorithm();
    }
  }

  if (algorithmState == AlgorithmState::Stepping && ImGui::Button("Step")) {
    // stepAlgorithm();
  }
  if (algorithmState == AlgorithmState::Stepping && ImGui::Button("Run")) {
    algorithmState = AlgorithmState::Running;
  }
  if (algorithmState == AlgorithmState::Running && ImGui::Button("Pause")) {
    algorithmState = AlgorithmState::Stepping;
  }
  ImGui::End();

  DrawText("Congrats! You created your first window!", (width / 2) - 100,
           height / 2, 20, LIGHTGRAY);

#pragma region imgui
  rlImGuiEnd();

  if (g_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
#pragma endregion

  EndDrawing();
}

#include "raylib.h"
#include "raymath.h"
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
  var output = document.getElementById("output");
  output.hidden = !output.hidden;
})
EM_JS(int, canvas_set_size, (), {
  var canvas = document.getElementById("canvas");
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
  return window.innerWidth * window.innerHeight;
})

EM_JS(void, print, (const char *string),
      { Module.print(UTF8ToString(string)); })
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
Camera2D g_camera = {0};

Rectangle mouseCollider{0U, 0U, 15U, 15U};

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
Node_L *testNode = (Node_L *)malloc(sizeof(Node_L));

Node_L *selectedNode = nullptr;
Node_L *selectedNodeOrigin = nullptr;
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
  root->collider = {root->pos.x - root->radius, root->pos.y - root->radius, 15,
                    15};
  testNode->pos = {static_cast<float>(width) / 2.0f + 50.0f,
                   static_cast<float>(height) / 2.0f};
  testNode->radius = 10;
  testNode->collider = {testNode->pos.x - testNode->radius,
                        testNode->pos.y - testNode->radius, 15, 15};

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

  g_camera.zoom = 1.0f;
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
  // Calculate coordinates, sizes before drawing anything.
  if (!g_io->WantCaptureMouse) {
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / g_camera.zoom);
      g_camera.target = Vector2Add(g_camera.target, delta);
    }

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      // Get the world point that is under the mouse
      Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), g_camera);

      // Set the offset to where the mouse is
      g_camera.offset = GetMousePosition();

      // Set the target to match, so that the camera maps the world space point
      // under the cursor to the screen space point under the cursor at any zoom
      g_camera.target = mouseWorldPos;

      // Zoom increment
      // Uses log scaling to provide consistent zoom speed
      float scale = 0.2f * wheel;
      g_camera.zoom = Clamp(expf(logf(g_camera.zoom) + scale), 0.125f, 64.0f);
    }
    Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), g_camera);
    mouseCollider.x = mouseWorld.x - mouseCollider.width / 2;
    mouseCollider.y = mouseWorld.y - mouseCollider.height / 2;

    if (CheckCollisionRecs(mouseCollider, root->collider) &&
        selectedNode == nullptr) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
#if defined(PLATFORM_WEB)
        print("selected");
#endif
        selectedNode = root;
      }
      // print("Collision");
    } else if (selectedNode != nullptr &&
               !CheckCollisionRecs(selectedNode->collider,
                                   testNode->collider)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        selectedNode = nullptr;
      }
    }
    if (selectedNode != nullptr) {
      Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), g_camera);
      selectedNode->pos = mouseWorld;
      selectedNode->collider.x =
          mouseWorld.x - selectedNode->collider.width / 2;
      selectedNode->collider.y =
          mouseWorld.y - selectedNode->collider.height / 2;
    }
  }
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

  BeginMode2D(g_camera);
  DrawRectangleRec(mouseCollider, BLUE);

  DrawRectangleRec(root->collider, GREEN);
  DrawRectangleRec(testNode->collider, RED);
  DrawLineEx({0U, 0U}, {static_cast<float>(width), static_cast<float>(height)},
             3, RED);
  DrawCircleLines(root->pos.x, root->pos.y, root->radius, RED);

  DrawText("Algorithm Visualizer", (width / 2) - 100, height / 2, 20,
           LIGHTGRAY);

  EndMode2D();

#pragma region imgui
  rlImGuiBegin();

  ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
  ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(),
                               ImGuiDockNodeFlags_PassthruCentralNode);
  ImGui::PopStyleColor(2);
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

  if (g_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }

  ImGui::End();
  rlImGuiEnd();

#pragma endregion

  EndDrawing();
}

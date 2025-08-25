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
EM_JS(void, print_float, (float string), { Module.print(string); })

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
Font g_font;
Rectangle mouseCollider{0U, 0U, 15U, 15U};
Vector2 mouseWorldPos = {0, 0};
void UpdateDrawFrame(void);

enum class InteractionMode {
  None, // can hover and delete
  NodeSelect,
  NodeCreate,
  EdgeCreate,
  EdgeEdit,
};
InteractionMode g_mode = InteractionMode::None;
bool im_gui_g_mode_pesist = false;
void HandleInput(void);
/**
 * States of the algorithm.
 * */
enum class AlgorithmState { Idle, Stepping, Running, Done };

AlgorithmState algorithmState = AlgorithmState::Idle;

std::queue<int> q;
std::vector<bool> visited;

struct Edge {
  size_t from;
  size_t to;
};
size_t hoveredEdgeIndex = SIZE_MAX;

struct Node {
  Vector2 pos;
  Vector2 oldPos;
  uint16_t radius;
  Rectangle collider;

  int64_t data;
  std::vector<int> edges;
};

// allocate array of nodes
std::vector<Node> nodes;
std::vector<Edge> edges;

Node *root = nullptr;

Node *selectedNode = nullptr;
Node *selectedNodeOrigin = nullptr;

/**
 * Helper functions.
 * */
bool IsMouseHoveringEdge(const Vector2 &mouse, const Vector2 &p1,
                         const Vector2 &p2, float thickness = 5.0f) {
  // Vector from p1 to p2
  Vector2 edge = Vector2Subtract(p2, p1);
  Vector2 mouseVec = Vector2Subtract(mouse, p1);

  float edgeLenSq = Vector2LengthSqr(edge);
  if (edgeLenSq == 0.0f)
    return false;

  // Project mouseVec onto edge (clamped to [0,1])
  float t = Vector2DotProduct(mouseVec, edge) / edgeLenSq;
  t = Clamp(t, 0.0f, 1.0f);

  // Closest point on the edge
  Vector2 closest = Vector2Add(p1, Vector2Scale(edge, t));

  // Distance from mouse to closest point
  float dist = Vector2Distance(mouse, closest);
  return dist <= thickness;
}

int main(void) {

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  g_font = LoadFont("./resources/font/alpha_beta.png");
#if defined(PLATFORM_WEB)
  printf("%d", canvas_set_size());

  emscripten_get_canvas_element_size("#canvas", &width, &height);
// printf("%d", width);
#endif

  InitWindow(width, height, "Algorithm Visualizer - raylib");
  Node newNode;
  newNode.radius = 15;

  newNode.pos = {(float)(width / 2), (float)(height / 2)};
  newNode.collider = {newNode.pos.x - newNode.radius,
                      newNode.pos.y - newNode.radius, (float)newNode.radius * 2,
                      (float)newNode.radius * 2};
  newNode.data = 0;

  nodes.push_back(newNode);
  root = &nodes[0];

#pragma region imgui
  rlImGuiSetup(true);

  imguiThemes::green();

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
  SetTargetFPS(60);

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

  // Update window dimensions
#if defined(PLATFORM_WEB)
  emscripten_get_canvas_element_size("#canvas", &width, &height);
#elif defined(PLATFORM_DESKTOP)
  width = GetScreenWidth();
  height = GetScreenHeight();
#endif

  // Calculate coordinates, sizes before drawing anything.
  if (!g_io->WantCaptureMouse) {
    HandleInput();

    mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), g_camera);

    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      // Get the world point that is under the mouse

      g_camera.offset = GetMousePosition();

      // Set the target to match, so that the camera maps the world space point
      // under the cursor to the screen space point under the cursor at any zoom
      g_camera.target = mouseWorldPos;

      // Uses log scaling to provide consistent zoom speed
      float scale = 0.2f * wheel;
      g_camera.zoom = Clamp(expf(logf(g_camera.zoom) + scale), 0.125f, 64.0f);
    }
    mouseCollider.x = mouseWorldPos.x - mouseCollider.width / 2;
    mouseCollider.y = mouseWorldPos.y - mouseCollider.height / 2;
  }
  BeginDrawing();

  ClearBackground(RAYWHITE);

  BeginMode2D(g_camera);

  /*if (!g_io->WantCaptureMouse) {
    DrawRectangleRec(mouseCollider, BLUE);
  }*/
  for (size_t i = 0; i < edges.size(); i++) {
    Color col = (i == hoveredEdgeIndex) ? RED : DARKGRAY;
    DrawLineEx(nodes[edges[i].from].pos, nodes[edges[i].to].pos, 3, col);
  }
  if (selectedNodeOrigin != nullptr) {
    DrawLineEx(selectedNodeOrigin->pos,
               GetScreenToWorld2D(GetMousePosition(), g_camera), 3, RED);
  }

  for (size_t i = 0; i < nodes.size(); i++) {
    DrawCircleV(nodes[i].pos, nodes[i].radius,
                (selectedNode == &nodes[i]) ? BLUE
                : (root == &nodes[i])       ? GOLD
                                            : RED);

    // DrawCircleLines(nodes[i].pos.x, nodes[i].pos.y, nodes[i].radius,
    // BLACK); DrawRectangleRec(nodes[i].collider, YELLOW);
    char idText[10];
    sprintf(idText, "%d", (int)i);
    Vector2 textSize = MeasureTextEx(GetFontDefault(), idText, 20, 1);
    // DrawText(idText, nodes[i].pos.x - textSize.x / 2,
    //        nodes[i].pos.y - textSize.y / 2, 20, WHITE);
    DrawTextEx(GetFontDefault(), idText, nodes[i].pos - (textSize / 2), 20.0f,
               1.0f, WHITE);
  }

  EndMode2D();
  DrawTextEx(GetFontDefault(), "Algorithm Visualizer",
             {width / 2.0f - 150.0f, 100.0f}, 20.0f, 1.0f, LIGHTGRAY);

#pragma region imgui
  rlImGuiBegin();

  // Create docking space with proper flags
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
  ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, ImVec4(0, 0, 0, 0));
  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), dockspace_flags);
  ImGui::PopStyleColor(2);

  if (ImGui::Begin("Algorithm Visualizer")) {
    ImGui::Text("Settings");

#if defined(PLATFORM_WEB)
    if (ImGui::Button("Toggle Web Console")) {
      toggle_console();
    }
#endif

    if (algorithmState == AlgorithmState::Idle) {

      if (IsKeyPressed(KEY_A) || ImGui::Button("Add Node")) {
        g_mode = InteractionMode::NodeSelect;
        if (selectedNode == nullptr && selectedNodeOrigin == nullptr) {
          Node newNode;
          Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), g_camera);
          newNode.pos = mouseWorld;
          newNode.oldPos = mouseWorld;
          newNode.radius = 15;
          newNode.collider = {
              newNode.pos.x - newNode.radius, newNode.pos.y - newNode.radius,
              (float)newNode.radius * 2, (float)newNode.radius * 2};
          newNode.data = nodes.size();

          nodes.push_back(newNode);
          selectedNode = &nodes.back();
        }
      }
    }

    if (ImGui::Button("Start")) {
      algorithmState = AlgorithmState::Stepping;
      // resetAlgorithm();
    }
  }

  if (algorithmState == AlgorithmState::Stepping) {
    if (ImGui::Button("Step")) {
      // stepAlgorithm();
    }
    if (ImGui::Button("Run")) {
      algorithmState = AlgorithmState::Running;
    }
  }

  if (algorithmState == AlgorithmState::Running) {
    if (ImGui::Button("Pause")) {
      algorithmState = AlgorithmState::Stepping;
    }
  }

  const char *stateNames[] = {"Idle", "Stepping", "Running", "Done"};
  ImGui::Text("Current State: %s", stateNames[(int)algorithmState]);

  ImGui::Text("Nodes: %d", (int)nodes.size());
  ImGui::Text("Edges: %d", (int)edges.size());

  if (selectedNodeOrigin != nullptr) {
    ImGui::Text("Selected node for edge: %d",
                (int)(selectedNodeOrigin - &nodes[0]));
  }

  ImGui::End();

  rlImGuiEnd();
#pragma endregion

  EndDrawing();
}

#pragma region handling_input
void HandleInput(void) {

  switch (g_mode) {

  case InteractionMode::NodeSelect:
    // Handle node selection
    if (selectedNode != nullptr) {
      Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), g_camera);
      selectedNode->pos = mouseWorld;
      selectedNode->collider.x =
          mouseWorld.x - selectedNode->collider.width / 2;
      selectedNode->collider.y =
          mouseWorld.y - selectedNode->collider.height / 2;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      bool clickedOnNode = false;

      for (size_t i = 0; i < nodes.size(); i++) {

        if (CheckCollisionRecs(mouseCollider, nodes[i].collider)) {
          clickedOnNode = true;

          if (selectedNode == nullptr) {
            selectedNode = &nodes[i];
            nodes[i].oldPos = nodes[i].pos;
            break;
          } else if (selectedNode != &nodes[i]) {
            break;

          } else if (selectedNode == &nodes[i]) {
            // Deselect if clicking on the same node
            clickedOnNode = false;
            // selectedNode = nullptr;
          }
        }
      }

      // Deselect if clicking on empty space
      if (!clickedOnNode && selectedNode != nullptr) {
        selectedNode = nullptr;
      }
    }

    break;

  case InteractionMode::EdgeCreate:
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionRecs(mouseCollider, nodes[i].collider)) {
          if (selectedNodeOrigin == nullptr) {
            g_mode = InteractionMode::EdgeCreate;
            selectedNodeOrigin = &nodes[i];

            return;
          } else if (selectedNodeOrigin != nullptr &&
                     selectedNodeOrigin != &nodes[i]) {
            // create edge and set selectedNodeOrigin to null

            Edge newEdge;
            newEdge.from =
                selectedNodeOrigin - &nodes[0]; // Get index of origin node
            newEdge.to = i;                     // Index of destination node

            // Check if edge already exists
            bool edgeExists = false;
            for (const auto &edge : edges) {
              if ((edge.from == newEdge.from && edge.to == newEdge.to) ||
                  (edge.from == newEdge.to && edge.to == newEdge.from)) {
                edgeExists = true;
                break;
              }
            }

            if (!edgeExists) {
              edges.push_back(newEdge);

              // Also add to node's edge list
              nodes[newEdge.from].edges.push_back(newEdge.to);
              nodes[newEdge.to].edges.push_back(newEdge.from);
            }

            selectedNodeOrigin = nullptr;
          } else {
            // Clicked on the same node - deselect
            selectedNodeOrigin = nullptr;
          }
          return;
        }
      }
      selectedNodeOrigin = nullptr;
    }
    break;

  case InteractionMode::EdgeEdit:
    // Maybe highlight edge, allow delete
    for (size_t i = 0; i < edges.size(); i++) {
      if (IsMouseHoveringEdge(mouseWorldPos, nodes[edges[i].from].pos,
                              nodes[edges[i].to].pos)) {
        g_mode = InteractionMode::EdgeEdit;
        hoveredEdgeIndex = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
            selectedNodeOrigin == nullptr) {
          selectedNodeOrigin = &nodes[edges[i].from];
          std::vector<Edge>::iterator it = edges.begin();
          it += i;
          edges.erase(it);
          hoveredEdgeIndex = SIZE_MAX;
          return;
        }
      }
    }
    if (selectedNodeOrigin != nullptr &&
        IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionRecs(mouseCollider, nodes[i].collider)) {
          if (&nodes[i] != selectedNodeOrigin) {
            // create new edge
            Edge newEdge;
            int originNodeIdx = (int)(selectedNodeOrigin - &nodes[0]);
            newEdge.from = originNodeIdx;
            newEdge.to = i;
            edges.push_back(newEdge);

            nodes[newEdge.from].edges.push_back(newEdge.to);
            nodes[newEdge.to].edges.push_back(newEdge.from);
            selectedNodeOrigin = nullptr;
          }
        }
      }
    }

    break;

  case InteractionMode::None:
  default:
    // Camera panning, idle state

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionRecs(mouseCollider, nodes[i].collider)) {
          nodes[i].oldPos = nodes[i].pos;
          selectedNode = &nodes[i];
          g_mode = InteractionMode::NodeSelect;

          return;
        }
      }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

      for (size_t i = 0; i < nodes.size(); i++) {
        if (CheckCollisionRecs(mouseCollider, nodes[i].collider)) {
          g_mode = InteractionMode::EdgeCreate;
          selectedNodeOrigin = &nodes[i];

          return;
        }
      }
    }
    for (size_t i = 0; i < edges.size(); i++) {
      if (IsMouseHoveringEdge(mouseWorldPos, nodes[edges[i].from].pos,
                              nodes[edges[i].to].pos)) {
        hoveredEdgeIndex = i;
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
            selectedNodeOrigin == nullptr) {
          g_mode = InteractionMode::EdgeEdit;

          selectedNodeOrigin = &nodes[edges[i].from];
          std::vector<Edge>::iterator it = edges.begin();
          it += i;
          edges.erase(it);
          hoveredEdgeIndex = SIZE_MAX;
        }

        return;
      }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / g_camera.zoom);
      g_camera.target = Vector2Add(g_camera.target, delta);
    }

    break;
  }

  if (IsKeyPressed(KEY_ESCAPE)) {
    g_mode = InteractionMode::None;
    if (selectedNode != nullptr) {
      selectedNode->pos = selectedNode->oldPos;
      selectedNode->collider = {selectedNode->pos.x - selectedNode->radius,
                                selectedNode->pos.y - selectedNode->radius,
                                (float)selectedNode->radius * 2,
                                (float)selectedNode->radius * 2};
    }

    selectedNode = nullptr;
    selectedNodeOrigin = nullptr;
  }
}
#pragma endregion

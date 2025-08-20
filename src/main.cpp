#include "raylib.h"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include <iostream>

#pragma region imgui
#include "imgui.h"
#include "imguiThemes.h"
#include "rlImGui.h"
ImGuiIO *g_io = nullptr;
#pragma endregion

void UpdateDrawFrame(void);
int main(void) {

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 450, "raylib [core] example - basic window");

#pragma region imgui
  rlImGuiSetup(true);

  // you can use whatever imgui theme you like!
  // ImGui::StyleColorsDark();
  // imguiThemes::yellow();
  // imguiThemes::gray();
  imguiThemes::green();
  // imguiThemes::red();
  // imguiThemes::embraceTheDarkness();

  g_io = &ImGui::GetIO();
  // ImGuiIO &io = ImGui::GetIO(); (void)io;
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

#pragma region imgui
  rlImGuiBegin();

  ImGui::PushStyleColor(ImGuiCol_WindowBg, {});
  ImGui::PushStyleColor(ImGuiCol_DockingEmptyBg, {});
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  ImGui::PopStyleColor(2);
#pragma endregion

  ImGui::Begin("Test");

  ImGui::Text("Hello");
  ImGui::Button("Button");
  ImGui::Button("Button2");

  ImGui::End();

  DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);

#pragma region imgui
  rlImGuiEnd();

  if (g_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }
#pragma endregion
  EndDrawing();
}

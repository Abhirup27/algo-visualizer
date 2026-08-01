#include "utils.hpp"
#if defined(PLATFORM_WEB)
#include <emscripten/html5.h>
#endif
#include "app.hpp"
#include <string>
#include <web.hpp>
int main() {
#if defined(PLATFORM_WEB)
  // emscripten_get_canvas_element_size("#canvas", &width, &height);
  //std::string str = std::to_string((int)width);
  //print_console(str.c_str());
#endif
  App app;
  initApp(&app, {.x = 0, .y = 0}, "pixantiqua.png");
  if (!app.instance) {
    // TRACELOG(LOG_FATAL, "Failed to create window. Exiting");
    return -1;
  }
#if defined(PLATFORM_WEB)
  // app->initWeb();
#else
  SetTargetFPS(60);

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    app.m_Run();
  }
#endif
  return 0;
}
